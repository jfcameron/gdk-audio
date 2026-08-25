// © Joseph Cameron - All Rights Reserved

#include <gdk/audio/exception.h>
#include <gdk/audio/insert_effect.h>

#include <algorithm>
#include <cmath>
#include <span>
#include <vector>

namespace gdk::audio
{
    namespace
    {
        [[nodiscard]] inline float sample_at(const void *const aPCM, const std::size_t aIndex,
            const pcm_format aFormat)
        {
            if (aFormat == pcm_format::signed_16)
                return static_cast<const short *>(aPCM)[aIndex] / 32768.0f;

            return (static_cast<const unsigned char *>(aPCM)[aIndex] - 128) / 128.0f;
        }

        inline void set_sample(void *const aPCM, const std::size_t aIndex, const float aValue,
            const pcm_format aFormat)
        {
            const auto clamped = std::max(-1.0f, std::min(1.0f, aValue));

            if (aFormat == pcm_format::signed_16)
            {
                static_cast<short *>(aPCM)[aIndex] = static_cast<short>(clamped * 32767.0f);

                return;
            }

            static_cast<unsigned char *>(aPCM)[aIndex] =
                static_cast<unsigned char>(clamped * 127.0f + 128.0f);
        }

        class lowpass_insert final : public insert_effect
        {
            audio_floating_point_type m_Coefficient;

            std::vector<float> m_Previous;

        public:
            void process(std::span<std::byte> aPCM, const pcm_format aFormat,
                const std::size_t aChannelCount) override
            {
                const auto aFrames = aPCM.size() / (aChannelCount * bytes_per_sample(aFormat));

                m_Previous.resize(aChannelCount, 0.0f);

                for (std::size_t frame = 0; frame < aFrames; ++frame)
                    for (std::size_t channel = 0; channel < aChannelCount; ++channel)
                    {
                        const auto i = frame * aChannelCount + channel;

                        m_Previous[channel] += m_Coefficient
                            * (sample_at(aPCM.data(), i, aFormat) - m_Previous[channel]);

                        set_sample(aPCM.data(), i, m_Previous[channel], aFormat);
                    }
            }

            void reset() override
            {
                std::fill(m_Previous.begin(), m_Previous.end(), 0.0f);
            }

            lowpass_insert(const audio_floating_point_type aCutoffHertz,
                const std::size_t aSampleRate)
            {
                if (!aSampleRate) throw exception("a lowpass needs a sample rate to be cut off "
                    "relative to");

                const auto rc = 1.0f / (2.0f * 3.14159265f * std::max(1.0f, aCutoffHertz));
                const auto dt = 1.0f / static_cast<float>(aSampleRate);

                m_Coefficient = std::min(1.0f, dt / (rc + dt));
            }
        };

        class echo_insert final : public insert_effect
        {
            std::vector<float> m_Delay;

            std::size_t m_Cursor = 0;

            audio_floating_point_type m_Feedback;
            audio_floating_point_type m_Mix;

        public:
            void process(std::span<std::byte> aPCM, const pcm_format aFormat,
                const std::size_t aChannelCount) override
            {
                const auto aFrames = aPCM.size() / (aChannelCount * bytes_per_sample(aFormat));

                if (m_Delay.empty()) return;

                for (std::size_t frame = 0; frame < aFrames; ++frame)
                    for (std::size_t channel = 0; channel < aChannelCount; ++channel)
                    {
                        const auto i = frame * aChannelCount + channel;
                        const auto tap = (m_Cursor * aChannelCount + channel) % m_Delay.size();

                        const auto dry = sample_at(aPCM.data(), i, aFormat);
                        const auto wet = m_Delay[tap];

                        m_Delay[tap] = dry + wet * m_Feedback;

                        set_sample(aPCM.data(), i, dry * (1 - m_Mix) + wet * m_Mix, aFormat);
                    }

                m_Cursor = (m_Cursor + aFrames) % std::max<std::size_t>(1, m_Delay.size());
            }

            void reset() override
            {
                std::fill(m_Delay.begin(), m_Delay.end(), 0.0f);

                m_Cursor = 0;
            }

            echo_insert(const audio_floating_point_type aDelaySeconds,
                const audio_floating_point_type aFeedback, const audio_floating_point_type aMix,
                const std::size_t aSampleRate, const std::size_t aChannelCount)
            : m_Delay(static_cast<std::size_t>(
                std::max(1.0f, aDelaySeconds * aSampleRate)) * std::max<std::size_t>(1, aChannelCount),
                0.0f)
            , m_Feedback(std::max(0.0f, std::min(0.99f, aFeedback)))
            , m_Mix(std::max(0.0f, std::min(1.0f, aMix)))
            {
                if (!aSampleRate) throw exception("an echo needs a sample rate to delay by");
            }
        };
    }

    insert_effect_shared_ptr_type make_lowpass_insert(const audio_floating_point_type aCutoffHertz,
        const std::size_t aSampleRate)
    {
        return insert_effect_shared_ptr_type(new lowpass_insert(aCutoffHertz, aSampleRate));
    }

    insert_effect_shared_ptr_type make_echo_insert(const audio_floating_point_type aDelaySeconds,
        const audio_floating_point_type aFeedback, const audio_floating_point_type aMix,
        const std::size_t aSampleRate, const std::size_t aChannelCount)
    {
        return insert_effect_shared_ptr_type(
            new echo_insert(aDelaySeconds, aFeedback, aMix, aSampleRate, aChannelCount));
    }
}
