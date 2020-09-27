// © 2019 Joseph Cameron - All Rights Reserved
#include <gdk/audio/openal_simple_sound.h>
#include <gdk/audio/stb_vorbis.h>

#include <AL/al.h>
#include <AL/alc.h>

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>
#include <string>

namespace gdk::audio
{ 
        std::vector<ALuint> openal_simple_sound::getAlBufferHandles()
        {
           return {m_ALBufferHandle.get()}; 
        }

        openal_simple_sound::openal_simple_sound(const std::string &aOggVorbisFileName) 
        : openal_simple_sound([&aOggVorbisFileName]() -> file_buffer_type
        {
            std::ifstream in(aOggVorbisFileName, std::ios::in | std::ios::binary);

            if (!in.is_open()) 
                throw std::invalid_argument("could not open file: " + std::string(aOggVorbisFileName));

            return {(std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>()};
        }())
        {}

        /// \brief constructs from an in-memory file buffer
        openal_simple_sound::openal_simple_sound(const file_buffer_type &aFileBuffer)
        : m_ALBufferHandle([&aFileBuffer]()
        {
            int channels, sample_rate;
            short *data;
            auto samples = stb_vorbis_decode_memory(&aFileBuffer.front(), aFileBuffer.size(), &channels, &sample_rate, &data);

            if (!samples) throw std::runtime_error("could not decode the ogg vorbis file buffer");

            std::vector<short> pcmBuffer(data, data + (2 * samples));
            
            ALuint newALBufferHandle;

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

            return newALBufferHandle;
        }(),
        [](const ALuint a)
        {
            alDeleteBuffers(1, &a);
        })
        {}
}

