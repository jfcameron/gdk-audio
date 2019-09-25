// © 2019 Joseph Cameron - All Rights Reserved
// learning how to stream decoded PCM data from an ogg vorbis and play it back within an openal context

#include <AL/al.h>
#include <AL/alc.h>

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>

//TODO: split out into separate module etc. blah blah... or consume directly as .c file?
#define STB_DEFINE
#define STB_ONLY
#include "stb_vorbis.c"

/// \brief Video Streaming example
int streaming_decode_main(int argc, char **argv)
{
    //////////// CONTEXT AND DEVICE
    ALCdevice *const device = []()
    {
        //if (!alcIsExtensionPresent(0, "ALC_ENUMERATION_EXT")) 
        {
            auto device_buffer = alcOpenDevice(0); 
            if (!device_buffer) throw std::runtime_error("could not initialize context on audio device");

            return device_buffer;
        }
        //else
        {
            //TODO: offer a choice of devices? -> once better idea of api, build RAII abstraction, 
            //ctor option will be device preference
        }
    }();

    ALCcontext *const context = [device]()
    {
        auto context_buffer = alcCreateContext(device, 0);

        if (!alcMakeContextCurrent(context_buffer)) throw std::runtime_error("could not make initial audio context current!");

        return context_buffer;
    }();

    if (const auto error = alGetError(); error != AL_NO_ERROR) throw std::runtime_error("error!!!");

    std::cout << "device is: " << alcGetString(device, ALC_DEVICE_SPECIFIER) << "\n";

    //////////// OGG VORBIS file to decoder
    int error = 0;
    stb_vorbis_alloc *alloc;
    stb_vorbis *vorbis = stb_vorbis_open_filename(argv[1], &error, alloc);

    if (argc != 2) throw std::invalid_argument("program requires 1 arg: the ogg file!");
    if (error != 0) throw std::invalid_argument(std::string("could not load file: ") + argv[1]);

    stb_vorbis_info info = stb_vorbis_get_info(vorbis);

    show_info(vorbis);

    unsigned int samples = stb_vorbis_stream_length_in_samples(vorbis);
    float seconds = stb_vorbis_stream_length_in_seconds(vorbis);

    std::cout << "# of samples in stream : " << samples << ", seconds: " << seconds << "\n";

    // Connecting data stream from vorbis decoder to the openal context
    //why not leave chunk sizE to impl?
    const int chunk = 65536;
    short *data = static_cast<decltype(data)>(malloc(sizeof(data) * chunk)); //correct?

    //Create two buffers for streaming
    ALuint buffer[2]; 
    alGenBuffers(2, &buffer[0]);

    //fill the buffers
    stb_vorbis_get_samples_short_interleaved(vorbis, info.channels, data, chunk);
    alBufferData(buffer[0], AL_FORMAT_STEREO16, data, chunk*sizeof(short), info.sample_rate);

    stb_vorbis_get_samples_short_interleaved(vorbis, info.channels, data, chunk);
    alBufferData(buffer[1], AL_FORMAT_STEREO16, data, chunk*sizeof(short), info.sample_rate);

    //gen source
    ALuint source;
    alGenSources(1, &source);

    //queue buffers to the sound source and play
    alSourceQueueBuffers(source, 2, buffer);
    alSourcePlay(source);

    // =-=--=-= play -=---=-==
    for(;;)//(ALint state; state == AL_PLAYING; alGetSourcei(source, AL_SOURCE_STATE, &state))
    {
        ALint processed;
        alGetSourcei(source, AL_BUFFERS_PROCESSED, &processed);

        if (processed)
        {
            ALuint which;
            alSourceUnqueueBuffers(source, 1, &which);

            if (int amount = stb_vorbis_get_samples_short_interleaved(vorbis, info.channels, data, chunk))
            {
                // load next chunk of data into the used buffer then requeue it
                alBufferData(which, AL_FORMAT_STEREO16, data, amount * 2 * sizeof(short), info.sample_rate);
                alSourceQueueBuffers(source, 1, &which);
            }
            else break; //reliable exit condition?
        }
    }

    //// cleanup TODO. move to unique ptrs with userdefined deleters
    stb_vorbis_close(vorbis);

    return EXIT_SUCCESS;
}

