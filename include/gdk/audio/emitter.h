// © Joseph Cameron - MIT License

#ifndef GDK_AUDIO_EMITTER_H
#define GDK_AUDIO_EMITTER_H

#include <gdk/audio/types.h>

#include <optional>

namespace gdk::audio
{
    /// \brief emits a sound at a 3d position within a scene.
    ///
    /// Positions and velocities are in the scene's space
    class emitter
    {
    public:
        using vector_type = audio_vector3_type;

        /// \brief change position, in the scene's space
        /// \warning a sound must be mono to be placed in space at all; a stereo or multichannel
        /// sound plays flat wherever it is put, as a speaker bed, unless it was made with a place
        /// for each channel, which then stand about this position. \see sound::channel_count,
        /// scene::make_emitter
        virtual void set_position(const vector_type &aPosition) = 0;

        [[nodiscard]] virtual vector_type position() const = 0;

        /// \brief how fast the emitter is moving, in units per second
        ///
        /// Used against the listener's velocity to produce a doppler shift. Left at zero, there is
        /// none.
        virtual void set_velocity(const vector_type &aVelocity) = 0;

        [[nodiscard]] virtual vector_type velocity() const = 0;

        /// \brief set pitch by scalar. 1 means no effect to the sample's pitch
        virtual void set_pitch(const audio_floating_point_type aPitch) = 0;

        [[nodiscard]] virtual audio_floating_point_type pitch() const = 0;

        /// \brief scales this emitter's volume. 1 is unattenuated, 0 is silent.
        ///
        /// Combined with the gain of the listener that hears it, so an emitter at 1 in a scene whose
        /// listener is at 0.5 is heard at 0.5.
        virtual void set_gain(const audio_floating_point_type aGain) = 0;

        [[nodiscard]] virtual audio_floating_point_type gain() const = 0;

        /// \brief which way a directional emitter points, in the scene's space
        ///
        /// Zero, the default, means the emitter is heard equally in every direction and the cone is
        /// ignored. Rotated into the listener's frame along with the position.
        virtual void set_direction(const vector_type &aDirection) = 0;

        [[nodiscard]] virtual vector_type direction() const = 0;

        /// \brief the shape of a directional emitter, in radians
        ///
        /// Inside the inner angle the emitter is heard at full gain, outside the outer angle at
        /// aOuterGain, and between the two it is interpolated. Both angles are the full width of the
        /// cone, not the half angle.
        /// \warning has no effect unless a direction is set
        virtual void set_cone(const audio_floating_point_type aInnerAngle,
            const audio_floating_point_type aOuterAngle,
            const audio_floating_point_type aOuterGain) = 0;

        /// \brief how this emitter's volume falls away with distance
        ///
        /// \param aReferenceDistance the distance at which gain is unattenuated
        /// \param aMaxDistance the distance the implementation's curve is shaped to end at
        /// \param aRolloffFactor scales how quickly the fall off happens. 0 disables it.
        virtual void set_attenuation(const audio_floating_point_type aReferenceDistance,
            const audio_floating_point_type aMaxDistance,
            const audio_floating_point_type aRolloffFactor) = 0;

        /// \brief check if the emitter is emitting a sound
        [[nodiscard]] virtual bool is_playing() const = 0;

        /// \brief play continuously, starting again each time the sound ends
        virtual void set_looping(const bool aLooping) = 0;

        [[nodiscard]] virtual bool is_looping() const = 0;

        /// \brief begins emitting the attached sound, from its start: the sound's stream is rewound
        /// \see pcm_stream::rewind for a stream that cannot be
        virtual void play() = 0;

        /// \brief stops playback, and lets go of whatever sound was queued but not yet heard: the
        /// next play() starts again from the sound's start
        virtual void stop() = 0;

        /// \brief stops emitting, keeping the place: resume() carries on from here, and what was
        /// queued but not yet heard is kept and heard first. Immediate, give or take the device's
        /// own period. Nothing if not playing.
        virtual void pause() = 0;

        /// \brief carries on from where pause() stopped. Unlike play(), never rewinds; nothing if
        /// not paused
        virtual void resume() = 0;

        [[nodiscard]] virtual bool is_paused() const = 0;

        /// \brief drops whatever has been read from the sound but not yet heard, and carries on
        /// with what its stream hands out next, without rewinding it: for a stream whose position
        /// has moved under it, a seek or a change of channel. Nothing if stopped.
        virtual void discard_queued() = 0;

        /// \brief how much of the sound has been heard since play(), in frames at the sound's own
        /// rate, the device's latency counted: the frame coming out of the speakers now, as nearly
        /// as the backend can say. A looping sound keeps counting past its end. Absent when stopped,
        /// or when the backend cannot tell.
        [[nodiscard]] virtual std::optional<double> playback_position() const = 0;

        virtual ~emitter() = default;
    };
}

#endif

