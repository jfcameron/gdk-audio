// © Joseph Cameron - All Rights Reserved

#ifndef GDK_AUDIO_EMITTER_H
#define GDK_AUDIO_EMITTER_H

#include <gdk/audio/types.h>
#include <gdk/audio/send_effect.h>

namespace gdk::audio
{
    struct lowpass_parameters final {
        //! overall level, 0 to 1
        audio_floating_point_type gain = 1.0f;

        //! level of the high frequencies, 0 to 1. This is the knob that does the muffling.
        audio_floating_point_type high_frequency_gain = 1.0f;
    };

    /// \brief emits a sound at a 3d position within a scene.
    ///
    /// Positions and velocities are in the scene's space
    class emitter
    {
    public:
        using vector_type = audio_vector3_type;

        /// \brief change position, in the scene's space
        /// \warn a sound must be mono to be placed in space at all; a stereo sound plays flat
        /// wherever it is put. \see sound::channel_count
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
        /// \warn has no effect unless a direction is set
        virtual void set_cone(const audio_floating_point_type aInnerAngle,
            const audio_floating_point_type aOuterAngle,
            const audio_floating_point_type aOuterGain) = 0;

        /// \brief how this emitter's volume falls away with distance
        ///
        /// \param aReferenceDistance the distance at which gain is unattenuated
        /// \param aMaxDistance where attenuation stops; what happens beyond it is the distance model's
        ///        business \see openal_policy::DISTANCE_MODEL
        /// \param aRolloffFactor scales how quickly the fall off happens. 0 disables it.
        virtual void set_attenuation(const audio_floating_point_type aReferenceDistance,
            const audio_floating_point_type aMaxDistance,
            const audio_floating_point_type aRolloffFactor) = 0;

        /// \brief muffle this emitter
        virtual void set_lowpass(const lowpass_parameters &aParameters) = 0;

        /// \brief remove the muffling
        virtual void clear_lowpass() = 0;

        /// \brief check if the emitter is emitting a sound
        [[nodiscard]] virtual bool is_playing() const = 0;

        /// \brief play continuously, starting again each time the sound ends
        virtual void set_looping(const bool aLooping) = 0;

        [[nodiscard]] virtual bool is_looping() const = 0;

        /// \brief begins emitting the attached sound
        virtual void play() = 0;

        /// \brief stops playback
        virtual void stop() = 0;

        virtual ~emitter() = default;
    };
}

#endif

