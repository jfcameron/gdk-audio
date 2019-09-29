// © 2019 Joseph Cameron - All Rights Reserved

#include <gdk/audio/openal_emitter.h>

namespace gdk::audio
{
    openal_emitter::openal_emitter(std::shared_ptr<openal_stream> pStream)
    : m_pStream(pStream)
    {
        alGenSources(1, &m_alSourceHandle);
    }

    void openal_emitter::play()
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
}

