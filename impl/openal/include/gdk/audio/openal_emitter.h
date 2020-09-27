// © 2019 Joseph Cameron - All Rights Reserved

#ifndef GDK_AUDIO_OPENAL_EMITTER_H
#define GDK_AUDIO_OPENAL_EMITTER_H

#include <gdk/audio/emitter.h>

#include <gdk/audio/openal_sound.h>

#include <jfc/memory/smart_handle.h>

#include <AL/al.h>
#include <AL/alc.h>

#include <memory>

namespace gdk::audio
{
    /// \brief root emitter type for openal impl.
    /// provides a common ancestor for collections etc that do not care about
    /// the specific differences between streamers (multi buffer, constant decoding) and simples (single buffer, 1 decode at sound ctor time)
    class openal_emitter : public emitter
    {
        jfc::memory::smart_handle<ALuint> m_alSourceHandle;

    protected:
        enum class state
        {
            playing,
            stopped
        }
        m_state = state::stopped;

        ALuint getSourceHandle();

    public:
        openal_emitter();

        bool isPlaying() override;

        virtual void update() = 0;
    };
}

#endif

