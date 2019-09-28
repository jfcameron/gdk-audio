// © 2019 Joseph Cameron - All Rights Reserved
#include <gdk/audio/openal_context.h>

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
}

