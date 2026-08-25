// © Joseph Cameron - All Rights Reserved

#include <gdk/audio/exception.h>
#include <gdk/audio/sound.h>
#include <gdk/audio/stb_vorbis.h>

#include <functional>
#include <memory>
#include <vector>

namespace gdk::audio
{
    namespace
    {
        using decoder_ptr_type = std::unique_ptr<stb_vorbis, std::function<void(stb_vorbis *const)>>;

        [[nodiscard]] decoder_ptr_type open_decoder(const std::vector<unsigned char> &aData)
        {
            int error = VORBIS__no_error;

            decoder_ptr_type pDecoder(
                aData.empty() ? nullptr
                    : stb_vorbis_open_memory(&aData[0], static_cast<int>(aData.size()), &error, nullptr),
                [](stb_vorbis *const p) { if (p) stb_vorbis_close(p); });

            if (!pDecoder || error != VORBIS__no_error) throw exception(
                "ogg vorbis data is badly formed; could not create a decoder");

            return pDecoder;
        }

        class vorbis_stream final : public pcm_stream
        {
            std::shared_ptr<const std::vector<unsigned char>> m_pData;

            decoder_ptr_type m_pDecoder;

            std::size_t m_ChannelCount;

        public:
            std::size_t read(std::span<std::byte> aOut) override
            {
                const auto frameCapacity = aOut.size() / (m_ChannelCount * sizeof(short));

                if (!frameCapacity) return 0;

                const auto frames = stb_vorbis_get_samples_short_interleaved(m_pDecoder.get(),
                    static_cast<int>(m_ChannelCount), reinterpret_cast<short *>(aOut.data()),
                    static_cast<int>(frameCapacity * m_ChannelCount));

                return frames > 0 ? static_cast<std::size_t>(frames) : 0;
            }

            void rewind() override
            {
                stb_vorbis_seek_start(m_pDecoder.get());
            }

            vorbis_stream(std::shared_ptr<const std::vector<unsigned char>> apData,
                const std::size_t aChannelCount)
            : m_pData(std::move(apData))
            , m_pDecoder(open_decoder(*m_pData))
            , m_ChannelCount(aChannelCount)
            {}
        };

        class vorbis_sound final : public sound
        {
            std::shared_ptr<const std::vector<unsigned char>> m_pData;

            std::size_t m_ChannelCount;
            std::size_t m_SampleRate;
            audio_floating_point_type m_Duration;

        public:
            std::size_t channel_count() const override { return m_ChannelCount; }

            std::size_t sample_rate() const override { return m_SampleRate; }

            pcm_format format() const override { return pcm_format::signed_16; }

            audio_floating_point_type duration() const override { return m_Duration; }

            std::unique_ptr<pcm_stream> open() const override
            {
                return std::unique_ptr<pcm_stream>(new vorbis_stream(m_pData, m_ChannelCount));
            }

            vorbis_sound(std::vector<unsigned char> aData)
            : m_pData(std::make_shared<const std::vector<unsigned char>>(std::move(aData))) {
                const auto pDecoder = open_decoder(*m_pData);

                const auto info = stb_vorbis_get_info(pDecoder.get());

                m_ChannelCount = static_cast<std::size_t>(info.channels);
                m_SampleRate = static_cast<std::size_t>(info.sample_rate);
                m_Duration = stb_vorbis_stream_length_in_seconds(pDecoder.get());
            }
        };
    }

    sound_shared_ptr_type make_vorbis_sound(const unsigned char *const aData, const std::size_t aSize)
    {
        const auto size = aData ? aSize : 0;

        return sound_shared_ptr_type(new vorbis_sound(std::vector<unsigned char>(aData, aData + size)));
    }
}
