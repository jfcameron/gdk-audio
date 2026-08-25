// © Joseph Cameron - All Rights Reserved

#ifndef GDK_AUDIO_NULL_LISTENER_H
#define GDK_AUDIO_NULL_LISTENER_H

#include <gdk/audio/listener.h>

namespace gdk::audio {
    /// \brief a listener that hears nothing and remembers everything
    class null_listener final : public listener {
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

    private:
        audio_vector3_type mPosition{0, 0, 0};
        audio_quaternion_type mRotation;
        audio_vector3_type mVelocity{0, 0, 0};
        audio_floating_point_type mGain{1};
    };
}

#endif
