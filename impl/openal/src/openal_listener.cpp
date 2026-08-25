// © Joseph Cameron - All Rights Reserved

#include <gdk/audio/openal_listener.h>

namespace gdk::audio
{
    void openal_listener::set_position(const audio_vector3_type &aPosition) {
        m_Position = aPosition;
    }

    audio_vector3_type openal_listener::position() const {
        return m_Position;
    }

    void openal_listener::set_rotation(const audio_quaternion_type &aRotation) {
        m_Rotation = aRotation;
    }

    audio_quaternion_type openal_listener::rotation() const {
        return m_Rotation;
    }

    void openal_listener::set_transform(const audio_vector3_type &aPosition,
        const audio_quaternion_type &aRotation) {
        m_Position = aPosition;
        m_Rotation = aRotation;
    }

    void openal_listener::set_velocity(const audio_vector3_type &aVelocity) {
        m_Velocity = aVelocity;
    }

    audio_vector3_type openal_listener::velocity() const {
        return m_Velocity;
    }

    void openal_listener::set_gain(const audio_floating_point_type aGain) {
        m_Gain = aGain;
    }

    audio_floating_point_type openal_listener::gain() const {
        return m_Gain;
    }
}
