// © Joseph Cameron - All Rights Reserved

#ifndef GDK_AUDIO_OPENAL_LISTENER_H
#define GDK_AUDIO_OPENAL_LISTENER_H

#include <gdk/audio/types.h>
#include <gdk/audio/listener.h>

namespace gdk::audio
{
    /// \brief openal implementation of listener
    class openal_listener final : public listener
    {
        audio_vector3_type m_Position = audio_vector3_type::zero;
        audio_quaternion_type m_Rotation = audio_quaternion_type::identity;
        audio_vector3_type m_Velocity = audio_vector3_type::zero;
        audio_floating_point_type m_Gain = 1;

    public:
        virtual void set_position(const audio_vector3_type &aPosition) override;

        [[nodiscard]] virtual audio_vector3_type position() const override;

        virtual void set_rotation(const audio_quaternion_type &aRotation) override;

        [[nodiscard]] virtual audio_quaternion_type rotation() const override;

        virtual void set_transform(const audio_vector3_type &aPosition,
            const audio_quaternion_type &aRotation) override;

        virtual void set_velocity(const audio_vector3_type &aVelocity) override;

        [[nodiscard]] virtual audio_vector3_type velocity() const override;

        virtual void set_gain(const audio_floating_point_type aGain) override;

        [[nodiscard]] virtual audio_floating_point_type gain() const override;

        virtual ~openal_listener() override = default;
    };
}

#endif
