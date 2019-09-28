// © 2019 Joseph Cameron - All Rights Reserved
// brainstorming the interface and raii layers for game centric audio library (3d scene etc)

//TODO think about hearing impaired support. likely gui would be subtitles with vectors pointing towards source. This would require getPosition, getMagntitude. Possibly getEffects, so you could subtitle "muffled" etc.

#include <gdk/audio/context.h>
#include <gdk/audio/emitter.h>
#include <gdk/audio/sound.h>

#include <gdk/audio/openal_context.h>

#include <AL/al.h>
#include <AL/alc.h>

#include <array>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>

//TODO: split out into separate module etc. blah blah... or consume directly as .c file?
#define STB_DEFINE
#define STB_ONLY
#include "stb_vorbis.c"

namespace gdk::audio
{
    /// \brief OpenAL implementation of sound
    /// TODO: streaming from file results in a NOT SIMPLE resource type. each instance must own its own data (decoder's decoded PCM buffers). Will have to split into a sibling, openal_stream proper should represent a single openal pcm buffer where all data is decoded upfront, thus becoming a sharable resource that does not incur significant per instance cost (additional buffer allocations)
    /// analogous to a vertex buffer in OpenGL
    /// \warn instances of stream are heavy. Each instance has its own decoder, which has two streaming buffers it fills with decoded PCM data from the encoded raw data. Together these are about 1/4mb wide.
    class openal_stream : public sound
    {
        using raw_file_type = std::vector<unsigned char>;
        using vorbis_decoder_pointer_type = std::unique_ptr<stb_vorbis, std::function<void(stb_vorbis *const)>>;
        using pcm_buffer_type = std::array<short, 16384>; // 2^14 is the smallest buffer size that doesnt cause playback failure with my test file. I dont understand how to choose buffer size, this will undoubtedly become a.. learning opportunity in the future.

        /// \brief entire file copied to memory. can be shared between stream instances
        std::shared_ptr<raw_file_type> m_pOggVorbisFileBuffer; 

        /// \brief decodes vorbis data to the PCM buffers, expanding the encoded data in small parts, until the entire file has been streamed.
        vorbis_decoder_pointer_type m_pDecoder;

        /// \brief contains metadata to do with vorbis file (channel count, legnth in seconds, etc.)
        stb_vorbis_info m_VorbisInfo;

        //static constexpr int chunk = 16384;//2^14 works. // sample decoding code used 65536 ie 2^16 when allocating pcm buffers. This results in 512kb PCM *2 buffers so 1mb per decoder instance. 2^14 results in 0.25mb total. Smaller buffers fail with my test input file. I expect a learning point around this coming in the future.
        pcm_buffer_type m_PCMBuffer;

        /// \brief handles to the albuffers we write PCMbuffer to as they are depleted then re-enqueued for playback
        std::array<ALuint, 2> m_alBufferHandles;

    public:
        decltype(m_alBufferHandles) getAlBufferHandles();

        /// \brief creates a new audio stream on the same encoded data.
        //clone

        ///\brief build from raw ogg vorbis file buffer
        openal_stream(const std::shared_ptr<raw_file_type> aOggVorbisData);

        ///\brief build from filename
        openal_stream(const std::string &aOggVorbisFileName);

        virtual ~openal_stream();
    };

    decltype(openal_stream::m_alBufferHandles) openal_stream::getAlBufferHandles()
    {
        return m_alBufferHandles;
    }

    openal_stream::~openal_stream()
    {
        std::cout << "deleted stream\n";
        alDeleteBuffers(m_alBufferHandles.size(), &m_alBufferHandles.front()); //wrong. use smart handle
    }

    openal_stream::openal_stream(const std::string &aOggVorbisFileName) : openal_stream([&aOggVorbisFileName]()
    {
        std::ifstream in(aOggVorbisFileName, std::ios::in | std::ios::binary);

        if (!in.is_open()) throw std::invalid_argument("could not open file: " + aOggVorbisFileName);
        
        return std::make_shared<raw_file_type>(std::vector<unsigned char>((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>()));
    }())
    {}

    openal_stream::openal_stream(const std::shared_ptr<raw_file_type> aOggVorbisData)
    : m_pOggVorbisFileBuffer(aOggVorbisData)
    , m_pDecoder({
        [&]()
        {
            int error;
            stb_vorbis *vorbis = stb_vorbis_open_memory(&(*m_pOggVorbisFileBuffer.get())[0], m_pOggVorbisFileBuffer.get()->size(), &error, nullptr);

            if (!vorbis || error != VORBIS__no_error) throw std::invalid_argument("ogg vorbis data is badly formed; could not create decoder"); //TODO: handle error enum in a more informative way. (pass along the specific error or at least decorate exception.what) 

            return vorbis;
        }()
        ,
        [](stb_vorbis *const p)
        {
            stb_vorbis_close(p);
        }
    })
    , m_VorbisInfo(stb_vorbis_get_info(m_pDecoder.get()))
    {
        alGenBuffers(m_alBufferHandles.size(), &m_alBufferHandles.front());

        const ALenum format = [](int channelCount)
        {
            switch(channelCount)
            {
                case 1: return AL_FORMAT_MONO16;
                case 2: return AL_FORMAT_STEREO16; // TODO: support 8s?
            }
            
            throw std::invalid_argument("unsupported channel count in ogg vorbis file");
        }(m_VorbisInfo.channels);

        for (const auto &current_handle : m_alBufferHandles)
        {
            stb_vorbis_get_samples_short_interleaved(m_pDecoder.get(), m_VorbisInfo.channels, &m_PCMBuffer.front(), m_PCMBuffer.size());
            alBufferData(current_handle, format, &m_PCMBuffer.front(), m_PCMBuffer.size() * sizeof(pcm_buffer_type::value_type), m_VorbisInfo.sample_rate);
        }

//=--==--=123=-12=-321=-3=-21-=21=-32-=123-=132=-132-
/*        //gen source -> source should be moved to separate abstraction. (move to emitter, that is where spacial state will come in)
        ALuint m_alSourceHandle;
        alGenSources(1, &m_alSourceHandle);

        //queue buffers to the sound m_alSourceHandle and play
        alSourceQueueBuffers(m_alSourceHandle, 2, &m_alBufferHandles.front());

        // =-=--=-= play -=---=-==
        alSourcePlay(m_alSourceHandle);

        for(;;)
        {
            ALint processed;
            alGetSourcei(m_alSourceHandle, AL_BUFFERS_PROCESSED, &processed);

            if (processed)
            {
                ALuint which;
                alSourceUnqueueBuffers(m_alSourceHandle, 1, &which);

                if (int amount = stb_vorbis_get_samples_short_interleaved(m_pDecoder.get(), m_VorbisInfo.channels, &m_PCMBuffer.front(), m_PCMBuffer.size()))
                {
                    // load next m_PCMBuffer.size() of data into the used buffer then requeue it
                    alBufferData(which, format, &m_PCMBuffer.front(), amount * 2 * sizeof(short), m_VorbisInfo.sample_rate);
                    alSourceQueueBuffers(m_alSourceHandle, 1, &which);
                }
                else break; //reliable exit condition?
            }
        }*/
    }

    /// \brief 3d space filled with listeners and emitters etc.
    class scene
    {
    public:
        virtual ~scene() = default;
    };

    class openal_scene : public scene
    {
        std::vector<emitter> m_pEmitters;
    public:
        
        
    };

    class openal_emitter : public emitter
    {
        ALuint m_alSourceHandle;

        //std::shared_ptr<> m_pSound;
        std::shared_ptr<openal_stream> m_pStream; //TODO: need to support fully decoded (nonstreamed) resource type. Think. common interface? abstract base probably.

    public:
        openal_emitter(std::shared_ptr<openal_stream> pStream)
        : m_pStream(pStream)
        {
            alGenSources(1, &m_alSourceHandle);
        }

        virtual void play() override
        {
            const auto handles(m_pStream->getAlBufferHandles());

            alSourceQueueBuffers(m_alSourceHandle, handles.size(), &handles.front());

            // =-=--=-= play -=---=-==
            alSourcePlay(m_alSourceHandle);

            for(;;)
            {
                ALint processed;
                alGetSourcei(m_alSourceHandle, AL_BUFFERS_PROCESSED, &processed);

                if (processed)
                {
                    ALuint which;
                    alSourceUnqueueBuffers(m_alSourceHandle, 1, &which);

                    //This part has to be refactored into stream I think. I guess a play method?
                    /*if (int amount = stb_vorbis_get_samples_short_interleaved(m_pDecoder.get(), m_VorbisInfo.channels, &m_PCMBuffer.front(), m_PCMBuffer.size()))
                    {
                        // load next m_PCMBuffer.size() of data into the used buffer then requeue it
                        alBufferData(which, format, &m_PCMBuffer.front(), amount * 2 * sizeof(short), m_VorbisInfo.sample_rate);
                        alSourceQueueBuffers(m_alSourceHandle, 1, &which);
                    }
                    else break; //reliable exit condition? yes.*/
                }
            }
        }
    };

    /// \brief hears emitters in a 3d scene
    class listener
    {
    public:
        virtual ~listener() = default;
    };
}

int main(int argc, char **argv)
{
    if (argc != 2) throw std::invalid_argument("program requires 1 paramter: path to an ogg vorbis file\n");

    auto pContext = std::unique_ptr<gdk::audio::context>(new gdk::audio::openal_context());

    {
        std::shared_ptr<gdk::audio::sound> pStream(new gdk::audio::openal_stream(argv[1]));//auto pStream = std::make_shared<gdk::audio::sound>(gdk::audio::openal_stream(argv[1]));

        auto pEmitter(std::unique_ptr<gdk::audio::emitter>(new gdk::audio::openal_emitter(std::dynamic_pointer_cast<gdk::audio::openal_stream>(pStream))));

        pEmitter->play();
    }

    return EXIT_SUCCESS;
}

