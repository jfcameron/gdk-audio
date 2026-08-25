// © Joseph Cameron - All Rights Reserved

#ifndef GDK_AUDIO_IMPL_CAPTURE_BUFFER_H
#define GDK_AUDIO_IMPL_CAPTURE_BUFFER_H

#include <gdk/audio/exception.h>
#include <gdk/audio/microphone.h>

#include <algorithm>
#include <cstddef>
#include <deque>
#include <vector>
#include <span>

namespace gdk::audio {
    [[nodiscard]] inline const microphone::request &validated(const microphone::request &aRequest) {
        if (!aRequest.channel_count) throw exception("a microphone with no channels cannot capture");

        if (!aRequest.sample_rate) throw exception("a microphone with no sample rate cannot capture");

        if (!aRequest.capacity_in_frames)
            throw exception("a microphone with no capacity cannot capture");

        return aRequest;
    }

    class capture_buffer final {
    public:
        capture_buffer(const std::size_t aCapacityInFrames, const std::size_t aBytesPerFrame,
            const bool aDropOldestWhenFull)
        : mCapacityInFrames(aCapacityInFrames)
        , mBytesPerFrame(aBytesPerFrame)
        , mDropOldestWhenFull(aDropOldestWhenFull)
        {}

        [[nodiscard]] bool append(const unsigned char *const aData, const std::size_t aByteCount) {
            if (aData && aByteCount) mBytes.insert(mBytes.end(), aData, aData + aByteCount);

            const auto capacity = mCapacityInFrames * mBytesPerFrame;

            if (mBytes.size() <= capacity) return false;

            const auto excess = mBytes.size() - capacity;

            mDroppedFrames += excess / mBytesPerFrame;

            if (mDropOldestWhenFull) {
                mBytes.erase(mBytes.begin(), mBytes.begin() + excess);

                return false;
            }

            mBytes.resize(capacity);

            return true;
        }

        [[nodiscard]] std::size_t available_in_frames() const { return mBytes.size() / mBytesPerFrame; }

        [[nodiscard]] bool capacity_reached() const {
            return available_in_frames() >= mCapacityInFrames;
        }

        [[nodiscard]] std::size_t dropped_frame_count() const { return mDroppedFrames; }

        [[nodiscard]] std::size_t capacity_in_frames() const { return mCapacityInFrames; }

        [[nodiscard]] std::size_t read(std::span<std::byte> aOut) {
            if (aOut.empty()) return 0;

            const auto frames = std::min(aOut.size() / mBytesPerFrame, available_in_frames());
            const auto bytes = frames * mBytesPerFrame;

            std::copy(mBytes.begin(), mBytes.begin() + bytes,
                reinterpret_cast<unsigned char *>(aOut.data()));

            mBytes.erase(mBytes.begin(), mBytes.begin() + bytes);

            return frames;
        }

        [[nodiscard]] std::vector<unsigned char> take() {
            std::vector<unsigned char> out(mBytes.begin(), mBytes.end());

            mBytes.clear();

            return out;
        }

        void clear() { mBytes.clear(); }

    private:
        std::deque<unsigned char> mBytes;

        std::size_t mCapacityInFrames;
        std::size_t mBytesPerFrame;
        std::size_t mDroppedFrames{0};

        bool mDropOldestWhenFull;
    };
}

#endif
