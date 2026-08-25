// © Joseph Cameron - All Rights Reserved

#include <gdk/audio/openal_microphone.h>

#include <gdk/audio/exception.h>
#include <gdk/audio/impl_capture_buffer.h>

#include <algorithm>
#include <vector>

using namespace gdk::audio;

namespace {
    /// \brief the ALC format for a channel count and sample size
    [[nodiscard]] ALCenum al_capture_format(const std::size_t aChannelCount,
        const pcm_format aFormat) {
        if (aChannelCount == 1)
            return aFormat == pcm_format::unsigned_8 ? AL_FORMAT_MONO8 : AL_FORMAT_MONO16;

        if (aChannelCount == 2)
            return aFormat == pcm_format::unsigned_8 ? AL_FORMAT_STEREO8 : AL_FORMAT_STEREO16;

        throw exception("openal captures one or two channels; " + std::to_string(aChannelCount)
            + " was asked for");
    }

    /// \brief how many frames the *device's own* ring holds
    [[nodiscard]] ALCsizei device_ring_frames(const std::size_t aSampleRate) {
        return static_cast<ALCsizei>(std::max<std::size_t>(aSampleRate / 10, 1024));
    }
}

openal_microphone::openal_microphone(const request &aRequest)
: mRequest(validated(aRequest))
, m_pDevice({
    [this]() {
        if (!alcIsExtensionPresent(nullptr, "ALC_EXT_CAPTURE"))
            throw exception("this openal build has no capture support (ALC_EXT_CAPTURE is absent)");

        auto *const pDevice = alcCaptureOpenDevice(
            mRequest.device_name.empty() ? nullptr : mRequest.device_name.c_str(),
            static_cast<ALCuint>(mRequest.sample_rate),
            al_capture_format(mRequest.channel_count, mRequest.format),
            device_ring_frames(mRequest.sample_rate));

        if (!pDevice) throw exception("could not open capture device \""
            + (mRequest.device_name.empty() ? std::string("(default)") : mRequest.device_name) + "\"");

        return pDevice;
    }(),
    [](ALCdevice *const p) {
        alcCaptureStop(p);
        alcCaptureCloseDevice(p);
    }})
, m_pCaptured(std::make_unique<capture_buffer>(mRequest.capacity_in_frames,
    mRequest.channel_count * bytes_per_sample(mRequest.format), mRequest.drop_oldest_when_full)) {
    if (const auto *const pName = alcGetString(m_pDevice.get(), ALC_CAPTURE_DEVICE_SPECIFIER))
        mDeviceName = pName;
}

openal_microphone::~openal_microphone() = default;

std::size_t openal_microphone::sample_rate() const { return mRequest.sample_rate; }

std::size_t openal_microphone::channel_count() const { return mRequest.channel_count; }

pcm_format openal_microphone::format() const { return mRequest.format; }

std::string_view openal_microphone::device_name() const { return mDeviceName; }

std::size_t openal_microphone::capacity_in_frames() const { return mRequest.capacity_in_frames; }

std::size_t openal_microphone::bytes_per_frame() const {
    return mRequest.channel_count * bytes_per_sample(mRequest.format);
}

void openal_microphone::start() {
    if (mCapturing) return;

    alcCaptureStart(m_pDevice.get());

    if (const auto error = alcGetError(m_pDevice.get()); error != ALC_NO_ERROR)
        throw exception("could not start capture");

    mCapturing = true;
}

void openal_microphone::stop() {
    if (!mCapturing) return;

    alcCaptureStop(m_pDevice.get());

    mCapturing = false;
}

bool openal_microphone::capturing() const { return mCapturing; }

void openal_microphone::update() {
    if (!mCapturing) return;

    ALCint available = 0;

    alcGetIntegerv(m_pDevice.get(), ALC_CAPTURE_SAMPLES, 1, &available);

    if (const auto error = alcGetError(m_pDevice.get()); error != ALC_NO_ERROR) {
        mCapturing = false;

        return;
    }

    if (available <= 0) return;

    std::vector<unsigned char> frames(static_cast<std::size_t>(available) * bytes_per_frame());

    alcCaptureSamples(m_pDevice.get(), frames.data(), available);

    if (m_pCaptured->append(frames.data(), frames.size())) stop();
}

std::size_t openal_microphone::available_in_frames() const {
    return m_pCaptured->available_in_frames();
}

bool openal_microphone::capacity_reached() const { return m_pCaptured->capacity_reached(); }

std::size_t openal_microphone::dropped_frame_count() const {
    return m_pCaptured->dropped_frame_count();
}

sound_shared_ptr_type openal_microphone::take() {
    const auto bytes = m_pCaptured->take();

    return make_pcm_sound(bytes.empty() ? nullptr : bytes.data(), bytes.size() / bytes_per_frame(),
        mRequest.channel_count, mRequest.sample_rate, mRequest.format);
}

std::size_t openal_microphone::read(std::span<std::byte> aOut) {
    return m_pCaptured->read(aOut);
}

void openal_microphone::discard() { m_pCaptured->clear(); }
