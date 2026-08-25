// © Joseph Cameron - All Rights Reserved

#include <gdk/audio/exception.h>
#include <gdk/audio/openal_simple_emitter.h>

#include <vector>
#include <span>

namespace gdk::audio
{
    openal_simple_emitter::openal_simple_emitter(const sound_shared_ptr_type &apSound,
        const openal_policy &aPolicy)
    : openal_emitter()
	, m_ALBufferHandle([&apSound, &aPolicy]()
	{
		const auto bytesPerFrame = apSound->channel_count() * bytes_per_sample(apSound->format());

		auto pStream = apSound->open();

		if (!pStream) throw exception("a sound handed back no stream to decode");

		std::vector<unsigned char> pcm;
		std::vector<unsigned char> chunk(aPolicy.DECODE_FRAMES_PER_READ * bytesPerFrame);

		while (const auto frames = pStream->read(std::as_writable_bytes(std::span(chunk))))
			pcm.insert(pcm.end(), chunk.begin(), chunk.begin() + (frames * bytesPerFrame));

		if (pcm.empty()) throw exception("a sound decoded to no audio at all");

		ALuint newALBufferHandle;

		alGenBuffers(1, &newALBufferHandle);

		alBufferData(newALBufferHandle,
			al_format_from(apSound->channel_count(), apSound->format()),
			pcm.data(),
			static_cast<ALsizei>(pcm.size()),
			static_cast<ALsizei>(apSound->sample_rate()));

		return newALBufferHandle;
	}(),
		[](const ALuint a)
	{
		alDeleteBuffers(1, &a);
	})
    {}

    void openal_simple_emitter::set_looping(const bool aLooping)
    {
        openal_emitter::set_looping(aLooping);

        alSourcei(source_handle(), AL_LOOPING, aLooping ? AL_TRUE : AL_FALSE);
    }

    void openal_simple_emitter::play()
    {
        const auto handle(m_ALBufferHandle.get());

		alSourceRewind(handle);

		alSourcei(source_handle(), AL_BUFFER, handle);

        alSourcePlay(source_handle());

        m_state = state::playing;
    }

    void openal_simple_emitter::update()
    {
        switch (m_state)
        {
            case state::playing:
            {
                const auto sourceHandle = source_handle();

                ALint processed;
                alGetSourcei(sourceHandle, AL_BUFFERS_PROCESSED, &processed);

                if (processed)
                {
                    ALuint which;
                    alSourceUnqueueBuffers(sourceHandle, 1, &which);

                    m_state = state::stopped;
                }
            } break;

            default: break;
        }
    }

	void openal_simple_emitter::stop()
	{
		alSourceStop(source_handle());

		m_state = state::stopped;
	}
}
