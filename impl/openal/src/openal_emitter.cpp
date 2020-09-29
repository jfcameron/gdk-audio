// © 2019 Joseph Cameron - All Rights Reserved

#include <gdk/audio/openal_emitter.h>
#include <gdk/audio/openal_stream.h>

#include <iostream>

namespace gdk::audio
{
    openal_emitter::openal_emitter()
		: m_alSourceHandle([]()
		{
			ALuint sourceHandle;

			alGenSources(1, &sourceHandle);

			return sourceHandle;
		}(),
		[](const ALuint a)
		{
			alDeleteSources(1, &a);
		})
    {}

    bool openal_emitter::isPlaying() const
    {
        return m_state == state::playing;
    }

    ALuint openal_emitter::getSourceHandle()
    {
        return m_alSourceHandle.get();
    }

	void openal_emitter::set_position(const vector_type& a)
	{
		m_Speed = a - m_LastPosition;
		m_Direction = vector_type(m_Speed).normalize();
		m_LastPosition = a;

		alSource3f(m_alSourceHandle.get(), AL_POSITION, a.x, a.y, a.z);
		//alSourcef(m_alSourceHandle.get(), AL_PITCH, 0.9f);

		//alListener3f(AL_POSITION, 0, 0, 0);
		//alListener3f(AL_VELOCITY, 0, 0, 0);
	}

	void openal_emitter::update()
	{
		//alSource3f(m_alSourceHandle.get(), AL_POSITION, a.x, a.y, a.z);
		//alSource3f(m_alSourceHandle.get(), AL_DIRECTION, a.x, a.y, a.z);
		//alSource3f(m_alSourceHandle.get(), AL_VELOCITY, a.x, a.y, a.z);
	}
}
