// © Joseph Cameron - All Rights Reserved

#ifndef GDK_AUDIO_NULL_MICROPHONE_H
#define GDK_AUDIO_NULL_MICROPHONE_H

#include <gdk/audio/microphone.h>

#include <gdk/audio/impl_capture_buffer.h>

#include <string>
#include <vector>

namespace gdk::audio {
    /// \brief a microphone a test speaks into
    class null_microphone final : public microphone {
    public:
        explicit null_microphone(request aRequest);

        [[nodiscard]] virtual std::size_t sample_rate() const override;
        [[nodiscard]] virtual std::size_t channel_count() const override;
        [[nodiscard]] virtual pcm_format format() const override;
        [[nodiscard]] virtual std::string_view device_name() const override;
        [[nodiscard]] virtual std::size_t capacity_in_frames() const override;

        virtual void start() override;
        virtual void stop() override;
        [[nodiscard]] virtual bool capturing() const override;
        virtual void update() override;

        [[nodiscard]] virtual std::size_t available_in_frames() const override;
        [[nodiscard]] virtual bool capacity_reached() const override;
        [[nodiscard]] virtual std::size_t dropped_frame_count() const override;
        [[nodiscard]] virtual sound_shared_ptr_type take() override;
        virtual std::size_t read(std::span<std::byte> aOut) override;
        virtual void discard() override;

        /// \brief what the device has heard, waiting for the next \ref update to collect it
        void offer(const void *const aData, const std::size_t aByteCount);

        //! \ref offer for the common case: aFrameCount frames of silence
        void offer_silence(const std::size_t aFrameCount);

        /// \brief the user refused permission, or the device went away
        void revoke();

    private:
        [[nodiscard]] std::size_t bytes_per_frame() const;

        request mRequest;

        std::string mDeviceName;

        std::vector<unsigned char> mPending;

        capture_buffer mCaptured;

        bool mCapturing{false};
    };
}

#endif
