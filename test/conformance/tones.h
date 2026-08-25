// © Joseph Cameron - All Rights Reserved

#ifndef GDK_AUDIO_TEST_TONES_H
#define GDK_AUDIO_TEST_TONES_H

#include <gdk/audio/sound.h>

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <numbers>
#include <vector>

[[nodiscard]] inline gdk::audio::sound_shared_ptr_type tone(const double aSeconds, const std::size_t aChannels = 1) {
    constexpr std::size_t RATE = 8000;
    constexpr double PITCH = 440;

    const auto frames = static_cast<std::size_t>(std::llround(aSeconds * RATE));

    std::vector<std::int16_t> samples(frames * aChannels);

    for (std::size_t frame = 0; frame < frames; ++frame) {
        const auto value = static_cast<std::int16_t>(
            std::lround(12000.0 * std::sin(2.0 * std::numbers::pi * PITCH * static_cast<double>(frame) / RATE)));

        for (std::size_t channel = 0; channel < aChannels; ++channel) samples[frame * aChannels + channel] = value;
    }

    return gdk::audio::make_pcm_sound(samples.data(), frames, aChannels, RATE, gdk::audio::pcm_format::signed_16);
}

[[nodiscard]] inline gdk::audio::sound_shared_ptr_type short_tone() { return tone(1.0); }

[[nodiscard]] inline gdk::audio::sound_shared_ptr_type long_tone() { return tone(6.0); }

[[nodiscard]] inline gdk::audio::sound_shared_ptr_type tiny_tone() { return tone(0.25); }

[[nodiscard]] inline gdk::audio::sound_shared_ptr_type stereo_tone() { return tone(1.0, 2); }

#endif
