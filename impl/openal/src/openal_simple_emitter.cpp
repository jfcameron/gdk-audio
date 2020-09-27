// © 2019 Joseph Cameron - All Rights Reserved

#include <gdk/audio/openal_simple_emitter.h>
#include <gdk/audio/openal_stream_emitter.h>

#include <iostream>

namespace gdk::audio
{
    openal_simple_emitter::openal_simple_emitter(std::shared_ptr<openal_simple_sound> pSimpleSound)
    : openal_emitter()
    , m_pSimpleStream(pSimpleSound)
    {}
    
    void openal_simple_emitter::play()
    {
        const auto handles(m_pSimpleStream->getAlBufferHandles()); //TODO: change soud tpy3e

        alSourceQueueBuffers(getSourceHandle()
            , handles.size()
            , &handles.front());

        alSourcePlay(getSourceHandle());

        m_state = state::playing;
    }
    
    void openal_simple_emitter::update()
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

                    m_state = state::stopped;
                }
            } break;

            default: break;
        }
    }
}

