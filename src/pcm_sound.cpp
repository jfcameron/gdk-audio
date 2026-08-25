// © Joseph Cameron - All Rights Reserved

#include <gdk/audio/exception.h>
#include <gdk/audio/sound.h>

#include <algorithm>
#include <cstring>
#include <memory>
#include <vector>

namespace gdk::audio {
    std::size_t bytes_per_sample(const pcm_format aFormat) {
        switch (aFormat) {
            case pcm_format::unsigned_8: return 1;
            case pcm_format::signed_16: return 2;
        }

        throw exception("unrecognized pcm format");
    }

    namespace {
        class pcm_buffer_stream final : public pcm_stream {
            std::shared_ptr<const std::vector<unsigned char>> m_pData;

            std::size_t m_BytesPerFrame;
            std::size_t m_FrameCount;
            std::size_t m_Cursor = 0;

        public:
            std::size_t read(std::span<std::byte> aOut) override {
                const auto frames = std::min(aOut.size() / m_BytesPerFrame, m_FrameCount - m_Cursor);

                if (frames) std::memcpy(aOut.data(), m_pData->data() + (m_Cursor * m_BytesPerFrame),
                    frames * m_BytesPerFrame);

                m_Cursor += frames;

                return frames;
            }

            void rewind() override { m_Cursor = 0; }

            pcm_buffer_stream(std::shared_ptr<const std::vector<unsigned char>> apData,
                const std::size_t aBytesPerFrame, const std::size_t aFrameCount)
            : m_pData(std::move(apData))
            , m_BytesPerFrame(aBytesPerFrame)
            , m_FrameCount(aFrameCount)
            {}
        };

        class pcm_sound final : public sound {
            std::shared_ptr<const std::vector<unsigned char>> m_pData;

            std::size_t m_FrameCount;
            std::size_t m_ChannelCount;
            std::size_t m_SampleRate;
            pcm_format m_Format;

        public:
            std::size_t channel_count() const override { return m_ChannelCount; }

            std::size_t sample_rate() const override { return m_SampleRate; }

            pcm_format format() const override { return m_Format; }

            audio_floating_point_type duration() const override {
                return static_cast<audio_floating_point_type>(m_FrameCount)
                    / static_cast<audio_floating_point_type>(m_SampleRate);
            }

            std::unique_ptr<pcm_stream> open() const override {
                return std::unique_ptr<pcm_stream>(new pcm_buffer_stream(m_pData,
                    m_ChannelCount * bytes_per_sample(m_Format), m_FrameCount));
            }

            pcm_sound(std::vector<unsigned char> aData, const std::size_t aFrameCount,
                const std::size_t aChannelCount, const std::size_t aSampleRate,
                const pcm_format aFormat)
            : m_pData(std::make_shared<const std::vector<unsigned char>>(std::move(aData)))
            , m_FrameCount(aFrameCount)
            , m_ChannelCount(aChannelCount)
            , m_SampleRate(aSampleRate)
            , m_Format(aFormat)
            {}
        };
    }

    sound_shared_ptr_type make_pcm_sound(const void *const aData, const std::size_t aFrameCount,
        const std::size_t aChannelCount, const std::size_t aSampleRate, const pcm_format aFormat) {
        if (!aChannelCount) throw exception("a sound with no channels cannot be played");

        if (!aSampleRate) throw exception("a sound with no sample rate cannot be played");

        if (aFrameCount && !aData) throw exception(
            "pcm data is null but a nonzero frame count was given");

        const auto bytes = aFrameCount * aChannelCount * bytes_per_sample(aFormat);

        const auto *const first = static_cast<const unsigned char *>(aData);

        return sound_shared_ptr_type(new pcm_sound(
            std::vector<unsigned char>(first, first + (aData ? bytes : 0)),
            aFrameCount, aChannelCount, aSampleRate, aFormat));
    }
}

