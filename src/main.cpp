// © 2019 Joseph Cameron - All Rights Reserved

#include <AL/al.h>
#include <AL/alc.h>

#include <stdexcept>
#include <iostream>

int main(int argc, char **argv)
{
    ALCdevice *const device = []()
    {
        //if (!alcIsExtensionPresent(0, "ALC_ENUMERATION_EXT")) 
        //{
            auto device_buffer = alcOpenDevice(NULL); 
            if (!device_buffer) throw std::runtime_error("could not initialize context on audio device");

            return device_buffer;
        //}
        //else
        //{
        // //TODO offer a choice of devices?
        //}
    }();

    ALCcontext *const context = [&device]()
    {
        auto context_buffer = alcCreateContext(device, NULL);

        if (!alcMakeContextCurrent(context_buffer)) throw std::runtime_error("could not make initial audio context current!");

        return context_buffer;
    }();

    if (const auto error = alGetError(); error != AL_NO_ERROR) throw std::runtime_error("error!!!");



    return 0;
}

