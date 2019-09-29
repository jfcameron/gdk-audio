// © 2019 Joseph Cameron - All Rights Reserved

#include <gdk/audio/openal_emitter.h>
#include <gdk/audio/openal_simple_sound.h>
#include <gdk/audio/openal_stream.h>

namespace gdk::audio
{
    openal_emitter::openal_emitter(std::shared_ptr<openal_sound> pStream)
    : m_pStream(pStream)
    , m_SoundType([&]()
    {
        if (std::dynamic_pointer_cast<openal_stream>(pStream)) 
            return decltype(m_SoundType)::stream;

        else if (std::dynamic_pointer_cast<openal_simple_sound>(pStream)) 
            return decltype(m_SoundType)::simple;

        throw std::invalid_argument("unsupported soundtype");
    }())
    , m_alSourceHandle([]()
    {
        ALuint sourceHandle;

        alGenSources(1, &sourceHandle);

        return sourceHandle;
    }(),
    [](const ALuint a)
    {
        alDeleteSources(1, &a);
    })
    {}

    void openal_emitter::play()
    {
        const auto handles(m_pStream->getAlBufferHandles());

        alSourceQueueBuffers(m_alSourceHandle.get(), handles.size(), &handles.front());

        alSourcePlay(m_alSourceHandle.get());

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
                const auto sourceHandle = m_alSourceHandle.get();

                ALint processed;
                alGetSourcei(sourceHandle, AL_BUFFERS_PROCESSED, &processed);

                if (processed)
                {
                    ALuint which;
                    alSourceUnqueueBuffers(sourceHandle, 1, &which);

                    if (m_SoundType == decltype(m_SoundType)::stream)
                    {
                        if (std::static_pointer_cast<openal_stream>(m_pStream)->decodeNextSamples(which))
                        {
                            alSourceQueueBuffers(sourceHandle, 1, &which);
                        }
                        else 
                        {
                            m_state = state::stopped;
                        }
                    }
                    else //simple case
                    {
                        m_state = state::stopped;
                    }
                }
            } break;

            default: break;
        }
    }
}

