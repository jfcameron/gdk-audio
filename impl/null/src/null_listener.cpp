// © Joseph Cameron - All Rights Reserved

#include <gdk/audio/null_listener.h>

using namespace gdk::audio;

void null_listener::set_position(const audio_vector3_type &aPosition) { mPosition = aPosition; }

audio_vector3_type null_listener::position() const { return mPosition; }

void null_listener::set_rotation(const audio_quaternion_type &aRotation) { mRotation = aRotation; }

audio_quaternion_type null_listener::rotation() const { return mRotation; }

void null_listener::set_transform(const audio_vector3_type &aPosition,
    const audio_quaternion_type &aRotation) {
    mPosition = aPosition;
    mRotation = aRotation;
}

void null_listener::set_velocity(const audio_vector3_type &aVelocity) { mVelocity = aVelocity; }

audio_vector3_type null_listener::velocity() const { return mVelocity; }

void null_listener::set_gain(const audio_floating_point_type aGain) { mGain = aGain; }

audio_floating_point_type null_listener::gain() const { return mGain; }
