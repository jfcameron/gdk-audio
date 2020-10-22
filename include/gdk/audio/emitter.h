// © 2019 Joseph Cameron - All Rights Reserved

#ifndef GDK_AUDIO_EMITTER_H
#define GDK_AUDIO_EMITTER_H

#include <functional>

#include <gdk/vector3.h>

namespace gdk::audio
{
    /// \brief emits a sound at a 3d position within the scene.
    class emitter
    {
    public:
		//! type used to store position of the emitter within the scene
		using vector_type = Vector3<float>;

		/// \brief change position
		/// \warn sound must be mono for this to have an effect
		virtual void set_position(const vector_type& aPosition) = 0;

		/// \brief set pitch 1.0f by scalar. 1.0 means no effect to the sample's pitch
		virtual void set_pitch(const float aPitch) = 0;

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

