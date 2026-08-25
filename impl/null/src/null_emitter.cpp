// © Joseph Cameron - All Rights Reserved

#include <gdk/audio/null_emitter.h>

using namespace gdk::audio;

void null_emitter::set_position(const vector_type &aPosition) { mPosition = aPosition; }
null_emitter::vector_type null_emitter::position() const { return mPosition; }

void null_emitter::set_velocity(const vector_type &aVelocity) { mVelocity = aVelocity; }
null_emitter::vector_type null_emitter::velocity() const { return mVelocity; }

void null_emitter::set_pitch(const audio_floating_point_type aPitch) { mPitch = aPitch; }
audio_floating_point_type null_emitter::pitch() const { return mPitch; }

void null_emitter::set_gain(const audio_floating_point_type aGain) { mGain = aGain; }
audio_floating_point_type null_emitter::gain() const { return mGain; }

void null_emitter::set_direction(const vector_type &aDirection) { mDirection = aDirection; }
null_emitter::vector_type null_emitter::direction() const { return mDirection; }

void null_emitter::set_cone(const audio_floating_point_type aInnerAngle,
    const audio_floating_point_type aOuterAngle, const audio_floating_point_type aOuterGain) {
    mInnerAngle = aInnerAngle;
    mOuterAngle = aOuterAngle;
    mOuterGain = aOuterGain;
}

void null_emitter::set_attenuation(const audio_floating_point_type aReferenceDistance,
    const audio_floating_point_type aMaxDistance, const audio_floating_point_type aRolloffFactor) {
    mReferenceDistance = aReferenceDistance;
    mMaxDistance = aMaxDistance;
    mRolloffFactor = aRolloffFactor;
}

void null_emitter::set_lowpass(const lowpass_parameters &) { mHasLowpass = true; }

void null_emitter::clear_lowpass() { mHasLowpass = false; }

bool null_emitter::is_playing() const { return mPlaying; }

void null_emitter::set_looping(const bool aLooping) { mLooping = aLooping; }

bool null_emitter::is_looping() const { return mLooping; }

void null_emitter::play() { mPlaying = true; }

void null_emitter::stop() { mPlaying = false; }

void null_emitter::finish() {
    if (!mLooping) mPlaying = false;
}

bool null_emitter::has_lowpass() const { return mHasLowpass; }

audio_floating_point_type null_emitter::inner_angle() const { return mInnerAngle; }
audio_floating_point_type null_emitter::outer_angle() const { return mOuterAngle; }
audio_floating_point_type null_emitter::outer_gain() const { return mOuterGain; }

audio_floating_point_type null_emitter::reference_distance() const { return mReferenceDistance; }
audio_floating_point_type null_emitter::max_distance() const { return mMaxDistance; }
audio_floating_point_type null_emitter::rolloff_factor() const { return mRolloffFactor; }
