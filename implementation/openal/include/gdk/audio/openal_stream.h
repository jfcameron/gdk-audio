// © 2019 Joseph Cameron - All Rights Reserved

#ifndef GDK_AUDIO_OPENAL_STREAM_H
#define GDK_AUDIO_OPENAL_STREAM_H

#include <gdk/audio/stb_vorbis.h>
#include <gdk/audio/sound.h>

#include <AL/al.h>
#include <AL/alc.h>

#include <array>
#include <vector>
#include <memory>

namespace gdk::audio
{
    /// \brief OpenAL implementation of sound
    /// \warn instances of stream are heavy. Each instance has its own decoder, which has two streaming buffers it fills with decoded PCM data from the encoded raw data. Together these are about 1/4mb wide.
        /// TODO: streaming from file results in a NOT SIMPLE resource type. each instance must own its own data (decoder's decoded PCM buffers). Will have to split into a sibling, openal_stream proper should represent a single openal pcm buffer where all data is decoded upfront, thus becoming a sharable resource that does not incur significant per instance cost (additional buffer allocations)
            /// analogous to a vertex buffer in OpenGL
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

        ALenum m_Format;

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

        ///\brief decodes the next chunk of data, writing to the specified AL buffer
        ///\warn this is an example of the statefulness of this object.
        /// return true indicates more data to play, emitter should continue
        /// return false indicates the decoder has reached end of vorbis data, emit should stop
        bool decodeNextSamples(ALuint aOutputPCMBuffer);

        virtual ~openal_stream();
    };
}

#endif

