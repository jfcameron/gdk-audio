// © 2019 Joseph Cameron - All Rights Reserved

#include <gdk/audio/openal_context.h>
#include <gdk/audio/openal_stream_emitter.h>
#include <gdk/audio/openal_simple_emitter.h>
#include <gdk/audio/stb_vorbis.h>

#include <iostream>
#include <fstream>
#include <string>

static inline std::string al_error_code_to_string(ALenum aError)
{
	switch (aError)
	{
		case AL_INVALID_NAME: return "AL_INVALID_NAME"; break;
		case AL_INVALID_ENUM: return "AL_INVALID_ENUM"; break;
		case AL_INVALID_VALUE: return "AL_INVALID_VALUE"; break;
		case AL_INVALID_OPERATION: return "AL_INVALID_OPERATION"; break;
		case AL_OUT_OF_MEMORY: return "AL_OUT_OF_MEMORY"; break;
	}

	throw std::invalid_argument(std::to_string(aError) + " is not a valid OpenAL error code");
}

namespace gdk::audio
{
    openal_context::openal_context()
    : m_pCurrentDevice({
        []()
        {
			auto device_buffer = alcOpenDevice(0); 

			if (!device_buffer) 
				throw std::runtime_error("could not initialize context on audio device");

			return device_buffer;
        }(),
        [](ALCdevice *const p)
        {
            alcCloseDevice(p);
        }})
    , m_pContext({
        [this]()
        {
            auto context_buffer = alcCreateContext(m_pCurrentDevice.get(), 0);

			alcProcessContext(context_buffer);

            if (!alcMakeContextCurrent(context_buffer)) 
				throw std::runtime_error("could not make initial audio context current");

            return context_buffer;
        }(), 
        [](ALCcontext *const p)
        {
            alcDestroyContext(p);
        }})
    {
        if (const auto error = alGetError(); error != AL_NO_ERROR) 
            throw std::runtime_error(al_error_code_to_string(error));
    }
    
    std::shared_ptr<sound> openal_context::make_sound(const sound::encoding_type aEncoding,
		sound::file_buffer_type&& aFileBuffer)
    {
		return std::shared_ptr<sound>(new openal_sound(aEncoding, aFileBuffer));
    }

    std::shared_ptr<emitter> openal_context::make_emitter(std::shared_ptr<sound> apSound)
    {
        std::shared_ptr<emitter> pEmitter;

		if (auto pSound = std::dynamic_pointer_cast<openal_sound>(apSound))
		{
			switch (pSound->getEncoding())
			{
			case sound::encoding_type::vorbis:
			{
				std::unique_ptr<stb_vorbis, std::function<void(stb_vorbis* const)>> pDecoder({[&]()
				{
					int error;

					stb_vorbis* vorbis = stb_vorbis_open_memory(&(pSound->getData())[0],
						pSound->getData().size(), &error, nullptr);

					if (!vorbis || error != VORBIS__no_error)
						throw std::invalid_argument("ogg vorbis data is badly formed; could not create decoder");

					return vorbis;
				}(),
				[](stb_vorbis* const p) { stb_vorbis_close(p); }});

				if (stb_vorbis_stream_length_in_seconds(pDecoder.get()) < 5)
				{
					pEmitter = std::unique_ptr<gdk::audio::openal_simple_emitter>(new openal_simple_emitter(pSound));
				}
				else
				{
					pEmitter = std::unique_ptr<gdk::audio::openal_stream_emitter>(new openal_stream_emitter(pSound));
				}
			} break;

			case sound::encoding_type::none:
			{
				pEmitter = std::unique_ptr<gdk::audio::openal_simple_emitter>(new openal_simple_emitter(pSound));
			} break;

			default: throw std::invalid_argument("tried to make an emitter using an unsupported encoding");
			}
		}
        else throw std::invalid_argument(
			"tried to make an emitter with a non-openal type. You cannot mix context implementations!");

        m_Emitters.push_back(std::static_pointer_cast<openal_emitter>(pEmitter));

        return pEmitter;
    }

    void openal_context::update()
    {
        for (std::remove_const<decltype(m_Emitters)::size_type>::type i(0); i < m_Emitters.size(); ++i)
        {
            if (m_Emitters[i].use_count() <= 1)
            {
                m_Emitters.erase(m_Emitters.begin() + i);
                
                --i;
            }
            else m_Emitters[i]->update();
        }
    }
}

