
/// \brief a microphone that writes into many buffers of fixed size
/// useful for generating audio data to be sent over a wire
//stream_microphone

#include <array>
/// \brief a microphone that writes to a single buffer
/// buffer writing begins at start_capture and ends at end_capture
/// useful for capturing a recording to be written to disk
class openal_simple_microphone
{
    ALubyte* captureBufPtr = nullptr;

    std::shared_ptr<ALCdevice> m_CaptureDevice = nullptr;
    ;
    ALint samplesCaptured = 0;

    std::vector<ALubyte> captureBuffer;

    public:
    openal_simple_microphone()
        : m_CaptureDevice(std::shared_ptr<ALCdevice>([]()
                    {
                    auto p = alcCaptureOpenDevice(nullptr, 8000, AL_FORMAT_MONO16, 800);

                    if (!p) throw std::runtime_error("could not open capture device");

                    return p;
                    }(),
                    [](ALCdevice* p)
                    {
                    alcCaptureCloseDevice(p);
                    }))
    {
        //TODO: provide an option if there are many mics
        //devices = alcGetString(NULL, ALC_CAPTURE_DEVICE_SPECIFIER);

        std::cout << captureBuffer.capacity() << "\n";

        captureBuffer.reserve(1048576);

        std::cout << captureBuffer.capacity() << "\n";
    }

    void start_capture()
    {
        alcCaptureStart(m_CaptureDevice.get());

        captureBufPtr = &captureBuffer.front();
    }

    void update()
    {
        ALint samplesAvailable;

        alcGetIntegerv(m_CaptureDevice.get(), ALC_CAPTURE_SAMPLES, 1, &samplesAvailable);

        if (samplesAvailable > 0)
        {
            //for (int i = 0; i < samplesAvailable * 4; i++) captureBuffer.push_back(0);

            alcCaptureSamples(m_CaptureDevice.get(), captureBufPtr, samplesAvailable);

            samplesCaptured += samplesAvailable;

            captureBufPtr += samplesAvailable * 2;
        }
    }

    std::shared_ptr<audio::sound> end_capture()
    {
        alcCaptureStop(m_CaptureDevice.get());

        captureBufPtr = &captureBuffer.front();

        auto pSound = std::shared_ptr<audio::sound>(
                new audio::openal_sound(audio::sound::encoding_type::none,
                    std::vector<unsigned char>(captureBufPtr, captureBufPtr + (samplesCaptured * 2))));

        return pSound;
    }
};

int main(void)
{
    auto pAudio = audio::context::make(audio::context::implementation::openal);

    std::shared_ptr<openal_simple_microphone> micky(new openal_simple_microphone());

    std::cout << "capture start\n";

    micky->start_capture();

    for (int i = 0; i < 300; ++i)
    {
        micky->update();

        std::this_thread::sleep_for(std::chrono::milliseconds(25));
    }

    auto pSound = micky->end_capture();

    auto pEmitter = pAudio->make_emitter(pSound);

    std::cout << "playback\n";

    pEmitter->play();

    while (pEmitter->isPlaying()) {}


    return EXIT_SUCCESS;
}

