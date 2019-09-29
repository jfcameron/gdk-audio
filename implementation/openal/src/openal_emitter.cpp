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

        alSourcePlay(m_alSourceHandle);

        m_state = state::playing;
    }

    bool openal_emitter::isPlaying()
    {
        return m_state == state::playing;
    }

    void openal_emitter::update()
    {
        switch (m_state)
        {
            case state::playing:
            {
                ALint processed;
                alGetSourcei(m_alSourceHandle, AL_BUFFERS_PROCESSED, &processed);

                if (processed)
                {
                    ALuint which;
                    alSourceUnqueueBuffers(m_alSourceHandle, 1, &which);

                    if (m_pStream->decodeNextSamples(which))
                    {
                        alSourceQueueBuffers(m_alSourceHandle, 1, &which);
                    }
                    else 
                    {
                        m_state = state::stopped;
                    }
                }
            } break;

            default: break;
        }
    }
}

