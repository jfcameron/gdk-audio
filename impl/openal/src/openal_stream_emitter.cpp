// © 2019 Joseph Cameron - All Rights Reserved

#include <gdk/audio/openal_stream_emitter.h>

#include <iostream>

namespace gdk::audio
{
    openal_stream_emitter::openal_stream_emitter(std::shared_ptr<openal_stream> pStream)
    : openal_emitter()
    , m_pStream(pStream)
    {}

    void openal_stream_emitter::play()
    {
        const auto handles(m_pStream->getAlBufferHandles());

        alSourceQueueBuffers(getSourceHandle()
            , handles.size()
            , &handles.front());

        alSourcePlay(getSourceHandle());

        m_state = state::playing;
    }
    
    void openal_stream_emitter::update()
    {
        switch (m_state)
        {
            case state::playing:
            {
                const auto sourceHandle = getSourceHandle();

                ALint processed;
                alGetSourcei(sourceHandle, AL_BUFFERS_PROCESSED, &processed);

                if (processed)
                {
                    ALuint which;
					alSourceUnqueueBuffers(sourceHandle, 1, &which);

                    if (m_pStream->decodeNextSamples(which))
                    {
                        alSourceQueueBuffers(sourceHandle, 1, &which);
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

