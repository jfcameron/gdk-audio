// © 2019 Joseph Cameron - All Rights Reserved

#ifndef GDK_AUDIO_EMITTER_H
#define GDK_AUDIO_EMITTER_H

namespace gdk::audio
{
    /// \brief emits a sound at a 3d position with effects
    /// has velocity relative to the current listener, creating doppler effect etc.
    class emitter
    {
    public:
        /// \brief emit the specified sound
        //virtual void start(sound *const pSound) = 0;

        /// \brief stops emitting sound
        //virtual void stop() = 0;
        //play(pSound) ?

        virtual void play() = 0;

        virtual ~emitter() = default;
    };
}

#endif

