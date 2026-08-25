// © Joseph Cameron - All Rights Reserved

#ifndef GDK_AUDIO_LISTENER_H
#define GDK_AUDIO_LISTENER_H

#include <gdk/audio/types.h>

namespace gdk::audio
{
    /// \brief hears the emitters in a scene: where the user is in that sound space.
    class listener
    {
    public:
        /// \brief where the listener is in scene space
        virtual void set_position(const audio_vector3_type &aPosition) = 0;

        [[nodiscard]] virtual audio_vector3_type position() const = 0;

        /// \brief which way the listener faces
        virtual void set_rotation(const audio_quaternion_type &aRotation) = 0;

        [[nodiscard]] virtual audio_quaternion_type rotation() const = 0;

        /// \brief set position and rotation in one call
        virtual void set_transform(const audio_vector3_type &aPosition,
            const audio_quaternion_type &aRotation) = 0;

        /// \brief how fast the listener is moving in units per second
        virtual void set_velocity(const audio_vector3_type &aVelocity) = 0;

        [[nodiscard]] virtual audio_vector3_type velocity() const = 0;

        /// \brief scales everything this listener hears. 1 is unattenuated, 0 is silent.
        virtual void set_gain(const audio_floating_point_type aGain) = 0;

        [[nodiscard]] virtual audio_floating_point_type gain() const = 0;

        virtual ~listener() = default;
    };
}

#endif

