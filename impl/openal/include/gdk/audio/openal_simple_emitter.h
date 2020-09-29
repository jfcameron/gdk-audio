// © 2019 Joseph Cameron - All Rights Reserved

#ifndef GDK_AUDIO_OPENAL_SIMPLE_EMITTER_H
#define GDK_AUDIO_OPENAL_SIMPLE_EMITTER_H

#include <gdk/audio/openal_emitter.h>
#include <gdk/audio/openal_sound.h>

#include <jfc/shared_handle.h>

#include <AL/al.h>
#include <AL/alc.h>

#include <memory>

namespace gdk::audio
{
	/// \brief emitter that entirely decodes the sound data
	/// rather than streaming it to buffers
	///
	/// todo: share decoded buffers
    class openal_simple_emitter : public openal_emitter
    {
	public:
		using sound_ptr_type = std::shared_ptr<openal_sound>;

	private:
		jfc::shared_handle<ALuint> m_ALBufferHandle;

    public:
        openal_simple_emitter(sound_ptr_type pSimpleSound);

        virtual void play() override;

		virtual void stop() override;
        
        virtual void update() override;
    };
}

#endif

