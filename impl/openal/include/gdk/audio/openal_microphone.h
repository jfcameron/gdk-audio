// © Joseph Cameron - All Rights Reserved

#ifndef GDK_AUDIO_OPENAL_MICROPHONE_H
#define GDK_AUDIO_OPENAL_MICROPHONE_H

#include <gdk/audio/microphone.h>

#include <AL/al.h>
#include <AL/alc.h>

#include <functional>
#include <memory>
#include <string>

namespace gdk::audio {
    class capture_buffer;

    /// \brief captures through OpenAL's ALC_EXT_CAPTURE
    class openal_microphone final : public microphone {
    public:
        explicit openal_microphone(const request &aRequest);

        virtual ~openal_microphone() override;

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
        [[nodiscard]] virtual sound_shared_ptr_type take() override;
        virtual std::size_t read(std::span<std::byte> aOut) override;
        virtual void discard() override;

        [[nodiscard]] virtual std::size_t dropped_frame_count() const override;

    private:
        [[nodiscard]] std::size_t bytes_per_frame() const;

        request mRequest;

        std::string mDeviceName;

        std::unique_ptr<ALCdevice, std::function<void(ALCdevice *const)>> m_pDevice;

        std::unique_ptr<capture_buffer> m_pCaptured;

        bool mCapturing{false};
    };
}

#endif
