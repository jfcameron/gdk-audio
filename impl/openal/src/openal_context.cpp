// © 2019 Joseph Cameron - All Rights Reserved

#include <gdk/audio/openal_context.h>
#include <gdk/audio/openal_simple_sound.h>
#include <gdk/audio/openal_stream.h>
#include <gdk/audio/openal_simple_emitter.h>
#include <gdk/audio/openal_stream_emitter.h>

#include <iostream>
#include <fstream>

namespace gdk::audio
{
    openal_context::openal_context()
    : m_pCurrentDevice({
        []()
        {
            //if (!alcIsExtensionPresent(0, "ALC_ENUMERATION_EXT")) 
            {
                auto device_buffer = alcOpenDevice(0); 
                if (!device_buffer) throw std::runtime_error("could not initialize context on audio device");
    
                //std::cout << "device is: " << alcGetString(device_buffer, ALC_DEVICE_SPECIFIER) << "\n";

                return device_buffer;
            }
            //else
            {
                //TODO: offer a choice of devices? -> once better idea of api, build RAII abstraction, 
                //ctor option will be device preference
            }
        }(),
        [](ALCdevice *const p)
        {
            alcCloseDevice(p);
        }})
    , m_pContext({
        [this]()
        {
            auto context_buffer = alcCreateContext(m_pCurrentDevice.get(), 0);

            if (!alcMakeContextCurrent(context_buffer)) throw std::runtime_error("could not make initial audio context current!");

            return context_buffer;
        }(), 
        [](ALCcontext *const p)
        {
            alcDestroyContext(p);
        }})
    {
        if (const auto error = alGetError(); error != AL_NO_ERROR) 
            throw std::runtime_error("error!!!"); //TODO: add magic enum dependency, use that to decorate?
    }
    
    std::shared_ptr<sound> openal_context::make_sound(audio_data dataView)
    {
		auto data = dataView.getData();

		if (dataView.getEncoding() != audio_data::encoding_type::vorbis) 
			throw std::runtime_error("gdk-graphics currently only supports vorbis encoded data");

        //TODO Should I split this out into  a factory? Flyweight strategy looks appropriate, but introduces a lot of state in the factory.
        //TODO decide when to stream or not. Probably has to do with expected ram usage.
        /*if (false) 
        {
            return std::shared_ptr<sound>(new openal_simple_sound(fileBuffer));
        }
        else*/ 
        {
			return std::shared_ptr<sound>(new openal_stream(data));
        }
    }

    std::shared_ptr<emitter> openal_context::make_emitter(std::shared_ptr<sound> apSound)
    {
        std::shared_ptr<emitter> pEmitter;

        if (auto pSimpleSound = std::dynamic_pointer_cast<openal_simple_sound>(apSound)) 
			pEmitter = std::unique_ptr<gdk::audio::openal_simple_emitter>(new openal_simple_emitter(pSimpleSound));
        else if (auto pStreamSound = std::dynamic_pointer_cast<openal_stream>(apSound)) 
			pEmitter = std::unique_ptr<gdk::audio::openal_stream_emitter>(new openal_stream_emitter(pStreamSound));
        else 
			throw std::invalid_argument("tried to make an emitter with a non-openal type. You cannot mix context implementations!");

        m_Emitters.push_back(std::static_pointer_cast<openal_emitter>(pEmitter));

        return pEmitter;
    }

    void openal_context::update()
    {
        for (std::remove_const<decltype(m_Emitters)::size_type>::type i(0); i < m_Emitters.size(); ++i)
        {
            if (m_Emitters[i].use_count() <= 1)
            {
                std::cout << "deleting " << i << "\n";
                m_Emitters.erase(m_Emitters.begin() + i);
                
                --i;
            }
            else m_Emitters[i]->update();
        }
    }
}

