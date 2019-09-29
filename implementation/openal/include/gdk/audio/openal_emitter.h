// © 2019 Joseph Cameron - All Rights Reserved

#ifndef GDK_AUDIO_OPENAL_EMITTER_H
#define GDK_AUDIO_OPENAL_EMITTER_H

#include <gdk/audio/emitter.h>

#include <gdk/audio/openal_stream.h>

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
        
        ALuint m_alSourceHandle;

        //std::shared_ptr<> m_pSound;
        std::shared_ptr<openal_stream> m_pStream;

    public:
        openal_emitter(std::shared_ptr<openal_stream> pStream);

        void play() override;

        bool isPlaying() override;

        ///
        void update();
    };
}

#endif

