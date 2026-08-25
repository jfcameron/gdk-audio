// © Joseph Cameron - All Rights Reserved

#include <gdk/audio/exception.h>
#include <gdk/audio/openal_emitter.h>

#include <gdk/math_constants.h>

#include <string>

namespace gdk::audio
{
    ALenum al_format_from(const std::size_t aChannelCount, const pcm_format aFormat)
    {
        if (aChannelCount == 1) return aFormat == pcm_format::unsigned_8
            ? AL_FORMAT_MONO8 : AL_FORMAT_MONO16;

        if (aChannelCount == 2) return aFormat == pcm_format::unsigned_8
            ? AL_FORMAT_STEREO8 : AL_FORMAT_STEREO16;

        throw exception("openal has no format for a sound with "
            + std::to_string(aChannelCount) + " channels");
    }

    openal_emitter::openal_emitter()
		: m_alSourceHandle([]()
		{
			ALuint sourceHandle;

			alGenSources(1, &sourceHandle);

			alSourcei(sourceHandle, AL_SOURCE_RELATIVE, AL_TRUE);

			return sourceHandle;
		}(),
		[](const ALuint a)
		{
			alDeleteSources(1, &a);
		})
    {}

    bool openal_emitter::is_playing() const
    {
        ALint state = AL_STOPPED;
        alGetSourcei(m_alSourceHandle.get(), AL_SOURCE_STATE, &state);

        return state == AL_PLAYING;
    }

    void openal_emitter::set_looping(const bool aLooping)
    {
        m_Looping = aLooping;
    }

    bool openal_emitter::is_looping() const
    {
        return m_Looping;
    }

    ALuint openal_emitter::source_handle() const
    {
        return m_alSourceHandle.get();
    }

	void openal_emitter::set_pitch(const audio_floating_point_type aPitch)
	{
		alSourcef(m_alSourceHandle.get(), AL_PITCH, aPitch);
	}

	audio_floating_point_type openal_emitter::pitch() const
	{
		ALfloat value = 1;
		alGetSourcef(m_alSourceHandle.get(), AL_PITCH, &value);

		return value;
	}

	void openal_emitter::set_gain(const audio_floating_point_type aGain)
	{
		m_Gain = aGain;
	}

	audio_floating_point_type openal_emitter::gain() const
	{
		return m_Gain;
	}

	void openal_emitter::set_direction(const vector_type &aDirection)
	{
		m_Direction = aDirection;
	}

	openal_emitter::vector_type openal_emitter::direction() const
	{
		return m_Direction;
	}

	void openal_emitter::set_cone(const audio_floating_point_type aInnerAngle,
		const audio_floating_point_type aOuterAngle, const audio_floating_point_type aOuterGain)
	{
		alSourcef(m_alSourceHandle.get(), AL_CONE_INNER_ANGLE, to_degrees(aInnerAngle));
		alSourcef(m_alSourceHandle.get(), AL_CONE_OUTER_ANGLE, to_degrees(aOuterAngle));
		alSourcef(m_alSourceHandle.get(), AL_CONE_OUTER_GAIN, aOuterGain);
	}

	void openal_emitter::set_attenuation(const audio_floating_point_type aReferenceDistance,
		const audio_floating_point_type aMaxDistance, const audio_floating_point_type aRolloffFactor)
	{
		alSourcef(m_alSourceHandle.get(), AL_REFERENCE_DISTANCE, aReferenceDistance);
		alSourcef(m_alSourceHandle.get(), AL_MAX_DISTANCE, aMaxDistance);
		alSourcef(m_alSourceHandle.get(), AL_ROLLOFF_FACTOR, aRolloffFactor);
	}

	void openal_emitter::set_position(const vector_type &aPosition)
	{
		m_Position = aPosition;
	}

	openal_emitter::vector_type openal_emitter::position() const
	{
		return m_Position;
	}

	void openal_emitter::set_velocity(const vector_type &aVelocity)
	{
		m_Velocity = aVelocity;
	}

	openal_emitter::vector_type openal_emitter::velocity() const
	{
		return m_Velocity;
	}

	void openal_emitter::set_lowpass(const lowpass_parameters &aParameters)
	{
		if (!m_pLowpassFilter) m_pLowpassFilter.reset(new jfc::shared_handle<ALuint>([]()
		{
			ALuint handle;

			alGenFilters(1, &handle);

			return handle;
		}(),
		[](const ALuint a) { alDeleteFilters(1, &a); }));

		const auto filter = m_pLowpassFilter->get();

		alFilteri(filter, AL_FILTER_TYPE, AL_FILTER_LOWPASS);
		alFilterf(filter, AL_LOWPASS_GAIN, aParameters.gain);
		alFilterf(filter, AL_LOWPASS_GAINHF, aParameters.high_frequency_gain);

		alSourcei(m_alSourceHandle.get(), AL_DIRECT_FILTER, static_cast<ALint>(filter));
	}

	void openal_emitter::clear_lowpass()
	{
		alSourcei(m_alSourceHandle.get(), AL_DIRECT_FILTER, AL_FILTER_NULL);

		m_pLowpassFilter.reset();
	}

	ALuint openal_emitter::lowpass_filter_handle() const
	{
		return m_pLowpassFilter ? m_pLowpassFilter->get() : static_cast<ALuint>(AL_FILTER_NULL);
	}

	void openal_emitter::route_to_send(const ALuint aEffectSlot)
	{
		alSource3i(m_alSourceHandle.get(), AL_AUXILIARY_SEND_FILTER,
			static_cast<ALint>(aEffectSlot), 0, AL_FILTER_NULL);

		m_SendSlot = aEffectSlot;
	}

	ALuint openal_emitter::send_slot() const
	{
		return m_SendSlot;
	}

	void openal_emitter::apply_listener(const listener &aListener)
	{
		const auto intoListenerSpace = aListener.rotation().inverse();

		const auto position = intoListenerSpace * (m_Position - aListener.position());

		const auto velocity = intoListenerSpace * (m_Velocity - aListener.velocity());

		const auto direction = intoListenerSpace * m_Direction;

		alSource3f(m_alSourceHandle.get(), AL_POSITION, position.x, position.y, position.z);
		alSource3f(m_alSourceHandle.get(), AL_VELOCITY, velocity.x, velocity.y, velocity.z);
		alSource3f(m_alSourceHandle.get(), AL_DIRECTION, direction.x, direction.y, direction.z);

		alSourcef(m_alSourceHandle.get(), AL_GAIN, m_Gain * aListener.gain());
	}

	void openal_emitter::update()
	{}
}
