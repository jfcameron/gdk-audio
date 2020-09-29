// © 2019 Joseph Cameron - All Rights Reserved

#ifndef GDK_AUDIO_EMITTER_H
#define GDK_AUDIO_EMITTER_H

#include <functional>

namespace gdk::audio
{
    /// \brief emits a sound at a 3d position with effects
    /// has velocity relative to the current listener, creating doppler effect etc.
    class emitter
    {
    public:
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

