// © 2019 Joseph Cameron - All Rights Reserved

#ifndef GDK_AUDIO_OPENAL_SIMPLE_EMITTER_H
#define GDK_AUDIO_OPENAL_SIMPLE_EMITTER_H

#include <gdk/audio/openal_emitter.h>
#include <gdk/audio/openal_simple_sound.h>

#include <jfc/memory/smart_handle.h>

#include <AL/al.h>
#include <AL/alc.h>

#include <memory>

namespace gdk::audio
{
    class openal_simple_emitter : public openal_emitter
    {
        std::shared_ptr<openal_simple_sound> m_pSimpleStream;

    public:
        openal_simple_emitter(std::shared_ptr<openal_simple_sound> pSimpleSound);

        virtual void play() override;
        
        virtual void update() override;
    };
}

#endif

