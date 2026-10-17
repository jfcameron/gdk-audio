// © Joseph Cameron - All Rights Reserved

#include <gdk/audio/exception.h>
#include <gdk/audio/impl_efx.h>
#include <gdk/audio/openal_emitter.h>

#include <gdk/math_constants.h>

#include <string>

namespace gdk::audio
{
    namespace {
        constexpr ALenum FORMAT_MONO_FLOAT32 = 0x10010;
        constexpr ALenum FORMAT_STEREO_FLOAT32 = 0x10011;
        constexpr ALenum FORMAT_QUAD8 = 0x1204, FORMAT_QUAD16 = 0x1205, FORMAT_QUAD32 = 0x1206;
        constexpr ALenum FORMAT_51CHN8 = 0x120A, FORMAT_51CHN16 = 0x120B, FORMAT_51CHN32 = 0x120C;
        constexpr ALenum FORMAT_61CHN8 = 0x120D, FORMAT_61CHN16 = 0x120E, FORMAT_61CHN32 = 0x120F;
        constexpr ALenum FORMAT_71CHN8 = 0x1210, FORMAT_71CHN16 = 0x1211, FORMAT_71CHN32 = 0x1212;

        constexpr ALenum SEC_OFFSET_LATENCY_SOFT = 0x1201;

        using get_sourcedv_soft_type = void (AL_APIENTRY *)(ALuint, ALenum, ALdouble *);

        [[nodiscard]] get_sourcedv_soft_type get_sourcedv_soft() {
            static const auto function = alIsExtensionPresent("AL_SOFT_source_latency")
                ? reinterpret_cast<get_sourcedv_soft_type>(alGetProcAddress("alGetSourcedvSOFT"))
                : nullptr;

            return function;
        }

        [[nodiscard]] ALenum pick(const pcm_format aFormat, const ALenum a8, const ALenum a16, const ALenum a32) {
            switch (aFormat) {
                case pcm_format::unsigned_8: return a8;
                case pcm_format::signed_16: return a16;
                case pcm_format::float_32: return a32;
            }

            throw exception("unrecognized pcm format");
        }
    }

    ALenum al_format_from(const std::size_t aChannelCount, const pcm_format aFormat)
    {
        if (aFormat == pcm_format::float_32 && !alIsExtensionPresent("AL_EXT_float32"))
            throw exception("this openal has no AL_EXT_float32, so a sound of float samples cannot be "
                "played; hand it over as signed 16 bit");

        if (aChannelCount > 2 && !alIsExtensionPresent("AL_EXT_MCFORMATS"))
            throw exception("this openal has no AL_EXT_MCFORMATS, so a sound with "
                + std::to_string(aChannelCount) + " channels cannot be played as one source; place each "
                "channel with scene::make_emitter's channel positions instead");

        switch (aChannelCount) {
            case 1: return pick(aFormat, AL_FORMAT_MONO8, AL_FORMAT_MONO16, FORMAT_MONO_FLOAT32);
            case 2: return pick(aFormat, AL_FORMAT_STEREO8, AL_FORMAT_STEREO16, FORMAT_STEREO_FLOAT32);
            case 4: return pick(aFormat, FORMAT_QUAD8, FORMAT_QUAD16, FORMAT_QUAD32);
            case 6: return pick(aFormat, FORMAT_51CHN8, FORMAT_51CHN16, FORMAT_51CHN32);
            case 7: return pick(aFormat, FORMAT_61CHN8, FORMAT_61CHN16, FORMAT_61CHN32);
            case 8: return pick(aFormat, FORMAT_71CHN8, FORMAT_71CHN16, FORMAT_71CHN32);
            default: break;
        }

        throw exception("openal has no format for a sound with "
            + std::to_string(aChannelCount) + " channels");
    }

    openal_emitter::openal_emitter(std::vector<vector_type> aChannelOffsets)
    : m_ChannelOffsets(aChannelOffsets.empty() ? std::vector<vector_type>{vector_type::zero} : std::move(aChannelOffsets))
    {
        m_alSourceHandles.reserve(m_ChannelOffsets.size());

        for (std::size_t i = 0; i < m_ChannelOffsets.size(); ++i) m_alSourceHandles.emplace_back([]()
        {
            ALuint sourceHandle;

            alGenSources(1, &sourceHandle);

            alSourcei(sourceHandle, AL_SOURCE_RELATIVE, AL_TRUE);

            return sourceHandle;
        }(),
        [](const ALuint a)
        {
            alDeleteSources(1, &a);
        });
    }

    bool openal_emitter::is_playing() const
    {
        for (const auto &handle : m_alSourceHandles)
        {
            ALint state = AL_STOPPED;
            alGetSourcei(handle.get(), AL_SOURCE_STATE, &state);

            if (state == AL_PLAYING) return true;
        }

        return false;
    }

    void openal_emitter::pause()
    {
        if (m_state != state::playing) return;

        before_pause();

        pause_sources();

        m_state = state::paused;
    }

    void openal_emitter::resume()
    {
        if (m_state != state::paused) return;

        play_sources();

        m_state = state::playing;
    }

    bool openal_emitter::is_paused() const
    {
        return m_state == state::paused;
    }

    void openal_emitter::play_sources()
    {
        const auto handles = source_handles();

        alSourcePlayv(static_cast<ALsizei>(handles.size()), handles.data());
    }

    void openal_emitter::pause_sources()
    {
        const auto handles = source_handles();

        alSourcePausev(static_cast<ALsizei>(handles.size()), handles.data());
    }

    void openal_emitter::stop_sources()
    {
        const auto handles = source_handles();

        alSourceStopv(static_cast<ALsizei>(handles.size()), handles.data());
    }

    std::pair<double, double> openal_emitter::source_offset_and_latency() const
    {
        if (const auto getSourcedv = get_sourcedv_soft())
        {
            ALdouble values[2] = {0, 0};

            getSourcedv(source_handle(), SEC_OFFSET_LATENCY_SOFT, values);

            if (alGetError() == AL_NO_ERROR) return {values[0], values[1]};
        }

        ALfloat offset = 0;
        alGetSourcef(source_handle(), AL_SEC_OFFSET, &offset);

        return {offset, 0};
    }

    bool openal_emitter::is_grouped() const
    {
        return m_Grouped;
    }

    void openal_emitter::set_grouped(const bool aGrouped)
    {
        m_Grouped = aGrouped;
    }

    bool openal_emitter::wants_to_play() const
    {
        return m_state != state::stopped;
    }

    void openal_emitter::note_paused()
    {
        if (m_state != state::playing) return;

        before_pause();

        m_state = state::paused;
    }

    void openal_emitter::note_resumed()
    {
        if (m_state == state::paused) m_state = state::playing;
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
        return m_alSourceHandles.front().get();
    }

    std::vector<ALuint> openal_emitter::source_handles() const
    {
        std::vector<ALuint> handles;

        handles.reserve(m_alSourceHandles.size());

        for (const auto &handle : m_alSourceHandles) handles.push_back(handle.get());

        return handles;
    }

	void openal_emitter::set_pitch(const audio_floating_point_type aPitch)
	{
		for (const auto &handle : m_alSourceHandles) alSourcef(handle.get(), AL_PITCH, aPitch);
	}

	audio_floating_point_type openal_emitter::pitch() const
	{
		ALfloat value = 1;
		alGetSourcef(source_handle(), AL_PITCH, &value);

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
		for (const auto &handle : m_alSourceHandles)
		{
			alSourcef(handle.get(), AL_CONE_INNER_ANGLE, to_degrees(aInnerAngle));
			alSourcef(handle.get(), AL_CONE_OUTER_ANGLE, to_degrees(aOuterAngle));
			alSourcef(handle.get(), AL_CONE_OUTER_GAIN, aOuterGain);
		}
	}

	void openal_emitter::set_attenuation(const audio_floating_point_type aReferenceDistance,
		const audio_floating_point_type aMaxDistance, const audio_floating_point_type aRolloffFactor)
	{
		for (const auto &handle : m_alSourceHandles)
		{
			alSourcef(handle.get(), AL_REFERENCE_DISTANCE, aReferenceDistance);
			alSourcef(handle.get(), AL_MAX_DISTANCE, aMaxDistance);
			alSourcef(handle.get(), AL_ROLLOFF_FACTOR, aRolloffFactor);
		}
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
		const auto *const pEfx = efx();

		if (!pEfx) throw exception("this openal has no effects extension (ALC_EXT_EFX), so an emitter "
			"cannot be muffled; an insert effect, make_lowpass_insert, filters without it");

		if (!m_LowpassFilter) m_LowpassFilter.reset([pEfx]()
		{
			ALuint handle;

			pEfx->alGenFilters(1, &handle);

			return handle;
		}(),
		[pEfx](const ALuint a) { pEfx->alDeleteFilters(1, &a); });

		const auto filter = m_LowpassFilter.get();

		pEfx->alFilteri(filter, EFX_FILTER_TYPE, EFX_FILTER_LOWPASS);
		pEfx->alFilterf(filter, EFX_LOWPASS_GAIN, aParameters.gain);
		pEfx->alFilterf(filter, EFX_LOWPASS_GAINHF, aParameters.high_frequency_gain);

		for (const auto &handle : m_alSourceHandles)
			alSourcei(handle.get(), EFX_DIRECT_FILTER, static_cast<ALint>(filter));
	}

	void openal_emitter::clear_lowpass()
	{
		if (!m_LowpassFilter) return;

		for (const auto &handle : m_alSourceHandles)
			alSourcei(handle.get(), EFX_DIRECT_FILTER, static_cast<ALint>(EFX_FILTER_NULL));

		m_LowpassFilter.reset();
	}

	ALuint openal_emitter::lowpass_filter_handle() const
	{
		return m_LowpassFilter.get();   
	}

	void openal_emitter::route_to_send(const ALuint aEffectSlot)
	{
		if (!efx()) return;

		for (const auto &handle : m_alSourceHandles)
			alSource3i(handle.get(), EFX_AUXILIARY_SEND_FILTER, static_cast<ALint>(aEffectSlot), 0,
				static_cast<ALint>(EFX_FILTER_NULL));

		m_SendSlot = aEffectSlot;
	}

	ALuint openal_emitter::send_slot() const
	{
		return m_SendSlot;
	}

	void openal_emitter::apply_listener(const listener &aListener)
	{
		const auto intoListenerSpace = aListener.rotation().inverse();

		const auto velocity = intoListenerSpace * (m_Velocity - aListener.velocity());

		const auto direction = intoListenerSpace * m_Direction;

		for (std::size_t i = 0; i < m_alSourceHandles.size(); ++i)
		{
			const auto position = intoListenerSpace * (m_Position + m_ChannelOffsets[i] - aListener.position());

			const auto handle = m_alSourceHandles[i].get();

			alSource3f(handle, AL_POSITION, position.x, position.y, position.z);
			alSource3f(handle, AL_VELOCITY, velocity.x, velocity.y, velocity.z);
			alSource3f(handle, AL_DIRECTION, direction.x, direction.y, direction.z);

			alSourcef(handle, AL_GAIN, m_Gain * aListener.gain());
		}
	}

	void openal_emitter::update()
	{}
}
