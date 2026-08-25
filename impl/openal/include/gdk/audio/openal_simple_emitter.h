// © Joseph Cameron - All Rights Reserved

#ifndef GDK_AUDIO_OPENAL_SIMPLE_EMITTER_H
#define GDK_AUDIO_OPENAL_SIMPLE_EMITTER_H

#include <gdk/audio/openal_emitter.h>
#include <gdk/audio/openal_policy.h>
#include <gdk/audio/sound.h>

#include <jfc/shared_handle.h>

#include <AL/al.h>
#include <AL/alc.h>

#include <memory>

namespace gdk::audio
{
	/// \brief emitter that decodes its sound once rather than streaming it
    class openal_simple_emitter : public openal_emitter
    {
		jfc::shared_handle<ALuint> m_ALBufferHandle;

    public:
        openal_simple_emitter(const sound_shared_ptr_type &apSound,
            const openal_policy &aPolicy);

        virtual void set_looping(const bool aLooping) override;

        virtual void play() override;

		virtual void stop() override;

        virtual void update() override;
    };
}

#endif
