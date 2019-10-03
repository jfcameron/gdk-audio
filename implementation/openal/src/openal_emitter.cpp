// © 2019 Joseph Cameron - All Rights Reserved

#include <gdk/audio/openal_emitter.h>
#include <gdk/audio/openal_simple_sound.h>
#include <gdk/audio/openal_stream.h>

#include <iostream>

namespace gdk::audio
{
    openal_emitter::openal_emitter()
    : m_alSourceHandle([]()
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

    bool openal_emitter::isPlaying()
    {
        return m_state == state::playing;
    }

    ALuint openal_emitter::getSourceHandle()
    {
        return m_alSourceHandle.get();
    }
}

