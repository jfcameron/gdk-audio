// © 2020 Joseph Cameron - All Rights Reserved

#include <gdk/audio/openal_simple_emitter.h>
#include <gdk/audio/stb_vorbis.h>

#include <string>
#include <iostream>

namespace gdk::audio
{
    openal_simple_emitter::openal_simple_emitter(sound_ptr_type pSimpleSound)
    : openal_emitter()
	,m_ALBufferHandle([&pSimpleSound]()
	{
		ALuint newALBufferHandle;

		switch (pSimpleSound->getEncoding())
		{
			case sound::encoding_type::vorbis:
			{
				auto pSound = std::static_pointer_cast<openal_sound>(pSimpleSound);

				auto aFileBuffer = pSound->getData();

				int channels, sample_rate;
				short* data;

				auto samples = stb_vorbis_decode_memory(&aFileBuffer.front(),
					aFileBuffer.size(),
					&channels,
					&sample_rate,
					&data); //Data is in the freestore; our responsibility to clean

				if (!samples) throw std::runtime_error("could not decode the ogg vorbis file buffer");

				std::vector<short> pcmBuffer(data, data + samples);

				free(data);

				alGenBuffers(1, &newALBufferHandle);

				alBufferData(newALBufferHandle
					, channels == 2
					? AL_FORMAT_STEREO16
					: channels == 1
					? AL_FORMAT_MONO16
					: throw std::invalid_argument("unsupported channel count: " + std::to_string(channels))
					, &pcmBuffer.front()
					, pcmBuffer.size() * sizeof(decltype(pcmBuffer)::value_type)
					, sample_rate);
			} break;

			case sound::encoding_type::none:
			{
				alGenBuffers(1, &newALBufferHandle);

				auto data = pSimpleSound->getData();

				alBufferData(newALBufferHandle
					, AL_FORMAT_MONO16 
					, &data.front()
					, data.size() * sizeof(decltype(data)::value_type)
					, 8000);
			} break;

			default: throw std::invalid_argument("simple emitter encountered unsupported encoding");
		}

		return newALBufferHandle;
	}(),
		[](const ALuint a)
	{
		alDeleteBuffers(1, &a);
	})
    {}
    
    void openal_simple_emitter::play()
    {
        const auto handle(m_ALBufferHandle.get()); //TODO: change soud tpy3e

		alSourceRewind(handle);

		alSourcei(getSourceHandle(), AL_BUFFER, handle);

        alSourcePlay(getSourceHandle());

        m_state = state::playing;
    }
    
    void openal_simple_emitter::update()
    {
        switch (m_state)
        {
            case state::playing:
            {
                const auto sourceHandle = getSourceHandle();

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
		alSourceStop(getSourceHandle());

		m_state = state::stopped;
	}
}
