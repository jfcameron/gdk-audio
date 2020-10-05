// © 2019 Joseph Cameron - All Rights Reserved
// learning how to decode an entire ogg vorbis file to PCM buffer
// compared to streaming:
// pro: fewer moving parts: encoded file buffer and vorbis decoder can be freed once decode is complete. In the streaming case, these two must persist, along with a continuous cpu cost as the vorbis data is decoded to PCM buffers and those prepared for playback. 
// con: PCM data is enormous. Only appropriate to do this on very small ogg vorbis files. If this is used on e.g a music track, initial decode will [on most modern machines] take an eternity (seconds), and consume a huge amount of memory (tens of megs? Possibly more?)
// thought: there is clearly a trade off point where one should be used rather than the other that has to do with memory usage. May be appropriate to hide these types behind a factory, returning one or the other depending on calculated ram usage.
#include <AL/al.h>
#include <AL/alc.h>

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>

#define STB_DEFINE
#define STB_ONLY
#include "stb_vorbis.c"

template<class T> using unique_delete_ptr = std::unique_ptr<T, std::function<void(T *const)>>;

/// \brief provides automatic based mechanism for cleanup of "handle types"
/// a handle is responsible for some sort of manual state managemnet at end of life. e.g: OpenGL/AL buffer handles, LibClang nodes
/// Deleter will only be called once: when final the copy of the smart handle goes out of scope.
/// Implementation based on final_action found in the Cpp General Guidelines, extended to strongly associate the handle with the deletor (both are members) and to support copy operations in addition to moves
/// analogous to stl shared_ptr
template<class handle_type>
class smart_handle final
{
public:
    using deleter_type = std::function<void(const handle_type)>;

private: 
    handle_type m_Handle;

    std::shared_ptr<deleter_type> m_pDeleter;

public:
    //TODO: enable/disable ref depending on handle_type? if sizeof(handle_type) < sizeof(handle_type *), no ref.
    const handle_type &get() const
    {
        return m_Handle;
    }

    smart_handle(handle_type &&aValue, deleter_type &&aDeleter)
    : m_Handle(aValue)
    , m_pDeleter(std::make_shared<smart_handle<handle_type>::deleter_type>(aDeleter))
    {}

    ~smart_handle()
    {
        if (m_pDeleter.use_count() == 1) (*m_pDeleter)(m_Handle);
    }
};

/// \brief Full file decode example
int main(int argc, char **argv)
{
    if (argc != 2) throw std::invalid_argument("requires exactly one argument: path to an ogg vorbis file");

    // Initialize OpenAL
    unique_delete_ptr<ALCdevice> pDevice(
        []()
        {
            auto pNewDevice = alcOpenDevice(0); 

            if (!pNewDevice) throw std::runtime_error("could not open the default audio device");

            return pNewDevice;
        }(),
        [](ALCdevice *const p)
        {
            alcCloseDevice(p);
        });

    unique_delete_ptr<ALCcontext> context(
        [](ALCdevice *const pDevice)
        {
            auto pNewContext = alcCreateContext(pDevice, 0);

            if (!pNewContext) throw std::runtime_error("could not initialize an OpenAL context");

            if (!alcMakeContextCurrent(pNewContext)) throw std::runtime_error("could not make initial OpenAL context current!");

            return pNewContext;
        }(pDevice.get()),
        [](ALCcontext *const p)
        {
            alcDestroyContext(p);
        });

    if (const auto error = alGetError(); error != AL_NO_ERROR) throw std::runtime_error("Could not initialize OpenAL");

    // Decode full vorbis data in ogg vorbis file to 16bit PCM buffer, hand the PCM over to the OpenAL context
    smart_handle<ALuint> alBufferHandle(
        [](const std::string &aOggVorbisFileName)
        {
            const std::vector<unsigned char> oggVorbisFileBuffer([](const std::string &aOggVorbisFileName) -> decltype(oggVorbisFileBuffer)
            {
                std::ifstream in(aOggVorbisFileName, std::ios::in | std::ios::binary);

                if (!in.is_open()) throw std::invalid_argument("could not open file: " + std::string(aOggVorbisFileName));
                
                return {(std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>()};
            }(aOggVorbisFileName));

            int channels, sample_rate;
            short *data;
            auto samples = stb_vorbis_decode_memory(&oggVorbisFileBuffer.front(), oggVorbisFileBuffer.size(), &channels, &sample_rate, &data);

            if (!samples) throw std::runtime_error("could not decode the ogg vorbis file");

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
        }(argv[1]),
        [](const ALuint a)
        {
            alDeleteBuffers(1, &a);
        });
    
    // Create a source to emit the sound somewhere in the OpenAL scene (by default at origin)
    smart_handle<ALuint> source(
        []()
        {
            ALuint newSource;

            alGenSources(1, &newSource);

            return newSource;
        }(),
        [](const ALuint a)
        {
            alDeleteSources(1, &a);
        });

    // Play the source, wait until playback is complete before exiting.
    alSourceQueueBuffers(source.get(), 1, &alBufferHandle.get());

    alSourcePlay(source.get());

    for (ALint processed(0); !processed; alGetSourcei(source.get(), AL_BUFFERS_PROCESSED, &processed));

    return EXIT_SUCCESS;
}

