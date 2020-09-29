// © 2020 Joseph Cameron - All Rights Reserved

#include <gdk/audio/openal_stream_emitter.h>

#include <iostream>
#include <string>
#include <vector>
#include <map>

using namespace gdk::audio;
    openal_stream_emitter::openal_stream_emitter(std::shared_ptr<openal_sound> pStream)
    : openal_emitter()
    , m_pSound(pStream)
	, m_pDecoder({
		[&]()
		{
			if (pStream->getEncoding() != sound::encoding_type::vorbis)
				throw std::runtime_error("openal_stream_emitter currently only supports vorbis encoded data");

			int error;

			stb_vorbis* vorbis = stb_vorbis_open_memory(&(pStream->getData())[0],
				pStream->getData().size(), &error, nullptr);

			//TODO: handle error enum in a more informative way; pass along stb error data
			if (!vorbis || error != VORBIS__no_error)
				throw std::invalid_argument("ogg vorbis data is badly formed; could not create decoder");

			return vorbis;
		}()
		,
		[](stb_vorbis* const p)
		{
			stb_vorbis_close(p);
		}})
	, m_VorbisInfo(stb_vorbis_get_info(m_pDecoder.get()))
	, m_alBufferHandles([&]()
		{
			decltype(m_alBufferHandles)::handle_type newBufferHandles;

			alGenBuffers(newBufferHandles.size(), &newBufferHandles.front());

			const ALenum format = m_Format = [](int channelCount)
			{
				switch (channelCount)
				{
					case 1: return AL_FORMAT_MONO16;
					case 2: return AL_FORMAT_STEREO16; // TODO: support 8s?
				}

				throw std::invalid_argument("unsupported channel count in ogg vorbis file");
			}(m_VorbisInfo.channels);

			return newBufferHandles;
		}(),
		[](const decltype(m_alBufferHandles)::handle_type a)
		{
			alDeleteBuffers(a.size(), &a.front());
		})
    {}

    void openal_stream_emitter::play()
    {
		if (m_state == state::stopped)
		{
			stb_vorbis_seek_start(m_pDecoder.get());

			for (size_t i(0); i < m_alBufferHandles.get().size(); ++i)
			{
				alSourceRewindv(m_alBufferHandles.get().size(), &m_alBufferHandles.get()[i]);

				decodeNextSamples(m_alBufferHandles.get()[i]);

				alSourceQueueBuffers(getSourceHandle(), 1, &m_alBufferHandles.get()[i]);
			}

			alSourcePlay(getSourceHandle());
			
			m_state = state::playing;
		}
    }
    
    void openal_stream_emitter::update()
    {
		const auto sourceHandle = getSourceHandle();

		ALint processed;
		alGetSourcei(sourceHandle, AL_BUFFERS_PROCESSED, &processed);

		if (processed)
		{
			ALuint which;
			alSourceUnqueueBuffers(sourceHandle, 1, &which);

			if (m_state == state::playing)
			if (decodeNextSamples(which))
			{
				alSourceQueueBuffers(sourceHandle, 1, &which);
			}
		}

		ALint queuedBufferCount;
		alGetSourcei(getSourceHandle(), AL_BUFFERS_QUEUED, &queuedBufferCount);
		
		if (!queuedBufferCount)
		{
			m_state = state::stopped;
		}
		else
		{
			ALint state;
			alGetSourcei(getSourceHandle(), AL_SOURCE_STATE, &state);

			if (state != AL_PLAYING)
			{
				alSourcePlay(getSourceHandle());
			}
		}
    }

	bool openal_stream_emitter::decodeNextSamples(ALuint aOutputPCMBuffer)
	{
		m_PCMBuffer.fill(0);

		if (int amount = stb_vorbis_get_samples_short_interleaved(m_pDecoder.get(),
			m_VorbisInfo.channels,
			&m_PCMBuffer.front(),
			m_PCMBuffer.size()))
		{
			alBufferData(aOutputPCMBuffer
				, m_Format
				, &m_PCMBuffer.front()
				, m_PCMBuffer.size() * sizeof(pcm_buffer_type::value_type)
				, m_VorbisInfo.sample_rate);

			return true;
		}
		
		return false;
	}

void openal_stream_emitter::stop()
{
	m_state = state::stopped;
}
