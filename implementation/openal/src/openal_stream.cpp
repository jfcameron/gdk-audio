// © 2019 Joseph Cameron - All Rights Reserved

#include <gdk/audio/openal_stream.h>

#include <iostream>
#include <fstream>

namespace gdk::audio
{

    decltype(openal_stream::m_alBufferHandles) openal_stream::getAlBufferHandles()
    {
        return m_alBufferHandles;
    }

    openal_stream::~openal_stream()
    {
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

        //TODO clean this up
        m_Format = format;

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

    bool openal_stream::decodeNextSamples(ALuint aOutputPCMBuffer)    
    {
        if (int amount = stb_vorbis_get_samples_short_interleaved(m_pDecoder.get(), 
                    m_VorbisInfo.channels, &m_PCMBuffer.front(), m_PCMBuffer.size()))
        {
            alBufferData(aOutputPCMBuffer
                    , m_Format
                    , &m_PCMBuffer.front()
                    , amount * 2 * sizeof(short)
                    , m_VorbisInfo.sample_rate);

            return true;
        }
        //else break; //reliable exit condition? yes.

        return false;
    }
}

