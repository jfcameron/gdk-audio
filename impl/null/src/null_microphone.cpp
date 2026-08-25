// © Joseph Cameron - All Rights Reserved

#include <gdk/audio/null_microphone.h>

#include <gdk/audio/exception.h>

#include <algorithm>
#include <vector>

using namespace gdk::audio;

null_microphone::null_microphone(request aRequest)
: mRequest(validated(aRequest))
, mDeviceName(mRequest.device_name.empty() ? "null default device" : mRequest.device_name)
, mCaptured(mRequest.capacity_in_frames,
    mRequest.channel_count * bytes_per_sample(mRequest.format), mRequest.drop_oldest_when_full)
{}

std::size_t null_microphone::sample_rate() const { return mRequest.sample_rate; }

std::size_t null_microphone::channel_count() const { return mRequest.channel_count; }

pcm_format null_microphone::format() const { return mRequest.format; }

std::string_view null_microphone::device_name() const { return mDeviceName; }

std::size_t null_microphone::capacity_in_frames() const { return mRequest.capacity_in_frames; }

std::size_t null_microphone::bytes_per_frame() const {
    return mRequest.channel_count * bytes_per_sample(mRequest.format);
}

void null_microphone::start() { mCapturing = true; }

void null_microphone::stop() {
    mCapturing = false;

    mPending.clear();
}

bool null_microphone::capturing() const { return mCapturing; }

void null_microphone::revoke() {
    mCapturing = false;
    mPending.clear();
}

void null_microphone::offer(const void *const aData, const std::size_t aByteCount) {
    if (!mCapturing || !aData) return;

    const auto *const first = static_cast<const unsigned char *>(aData);

    mPending.insert(mPending.end(), first, first + aByteCount);
}

void null_microphone::offer_silence(const std::size_t aFrameCount) {
    if (!mCapturing) return;

    const auto value = mRequest.format == pcm_format::unsigned_8
        ? static_cast<unsigned char>(128)
        : static_cast<unsigned char>(0);

    mPending.insert(mPending.end(), aFrameCount * bytes_per_frame(), value);
}

void null_microphone::update() {
    if (!mCapturing || mPending.empty()) return;

    if (mCaptured.append(mPending.data(), mPending.size())) mCapturing = false;

    mPending.clear();
}

std::size_t null_microphone::available_in_frames() const {
    return mCaptured.available_in_frames();
}

bool null_microphone::capacity_reached() const { return mCaptured.capacity_reached(); }

std::size_t null_microphone::dropped_frame_count() const {
    return mCaptured.dropped_frame_count();
}

sound_shared_ptr_type null_microphone::take() {
    const auto bytes = mCaptured.take();

    return make_pcm_sound(bytes.empty() ? nullptr : bytes.data(), bytes.size() / bytes_per_frame(),
        mRequest.channel_count, mRequest.sample_rate, mRequest.format);
}

std::size_t null_microphone::read(std::span<std::byte> aOut) {
    return mCaptured.read(aOut);
}

void null_microphone::discard() { mCaptured.clear(); }
