// © 2019 Joseph Cameron - All Rights Reserved

#ifndef GDK_AUDIO_CONTEXT_H
#define GDK_AUDIO_CONTEXT_H

#include <gdk/audio/sound.h>
#include <gdk/audio/emitter.h>
#include <gdk/audio/audio_data.h>

#include <functional>
#include <memory>

// TODO: non-static makes should return weak_ptrs, since emitted instances cannot exist outside of their context
// when a context falls out of scope, all its emissions should be cleaned. OpenAL is very stateful and requires this kind of collaboration to prevent leaks (cannot free bound buffer handles etc).
namespace gdk::audio
{
    /// \brief software connection to audio hardware, implementation global state goes here?
    /// active device etc?
    /// factories?
    class context
    {
    public:
		//! type returned by context factory
		using implementation_unique_ptr_type = std::unique_ptr<context>;

		//!
		using implementation_shared_ptr_type = std::shared_ptr<context>;

        /// \brief used to specify implementation behind context::make
		enum class implementation
		{
			openal
		};

        /// \brief factory method, creates a context of specified implementation
        static implementation_unique_ptr_type make(implementation impl);

        /// \brief builds a sound from a filebuffer
        /// \warn throws if the buffer does not contain encoded audio data of a type the context supports
        virtual std::shared_ptr<sound> make_sound(audio_data fileBuffer) = 0;
      
        /// \brief builds an emitter from a pointer to a sound
        virtual std::shared_ptr<emitter> make_emitter(std::shared_ptr<sound> aSound) = 0;

		//TODO: audio scene make

        /// \brief loop behaviour; must be called in your update loop for gdk::audio to behave correctly
        virtual void update() = 0;

        virtual ~context() = default;
    };
}

#endif

