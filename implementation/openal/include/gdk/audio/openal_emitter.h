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
    class openal_emitter : public emitter
    {
        enum class state
        {
            playing,
            stopped
        }
        m_state = state::stopped;
        
        jfc::memory::smart_handle<ALuint> m_alSourceHandle;

        std::shared_ptr<openal_sound> m_pStream;

        enum class sound_type
        {
            simple,
            stream
        } m_SoundType;

    public:
        openal_emitter(std::shared_ptr<openal_sound> pStream);

        void play() override;

        bool isPlaying() override;

        ///
        void update();
    };
}

#endif

