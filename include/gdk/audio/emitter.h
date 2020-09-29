// © 2019 Joseph Cameron - All Rights Reserved

#ifndef GDK_AUDIO_EMITTER_H
#define GDK_AUDIO_EMITTER_H

#include <functional>

#include <gdk/Vector3.h>

namespace gdk::audio
{
    /// \brief emits a sound at a 3d position within the scene.
    class emitter
    {
    public:
		//! type used to store position of the emitter within the scene
		using vector_type = Vector3<float>;

		/// \brief change
		virtual void set_position(const vector_type& aPosition) = 0;

		/// \brief check if the emitter is emitting a sound
		virtual bool isPlaying() const = 0;

        /// \brief begins emitting the attached sound TODO rename to emit?
        virtual void play() = 0;

		/// \brief stops playback
		virtual void stop() = 0;

        virtual ~emitter() = default;
    };
}

#endif

