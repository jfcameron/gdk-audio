// © Joseph Cameron - All Rights Reserved

#include <jfc/catch.hpp>

#include <gdk/audio/exception.h>
#include <gdk/audio/context.h>
#include <gdk/audio/openal_context.h>
#include <gdk/audio/openal_policy.h>
#include <gdk/audio/openal_simple_emitter.h>
#include <gdk/audio/openal_stream_emitter.h>
#include <gdk/audio/emitter.h>
#include <gdk/audio/scene.h>
#include <gdk/audio/sound.h>

#include "tones.h"

#include <AL/al.h>

#include <algorithm>
#include <cstdint>
#include <memory>
#include <vector>
#include <span>

using namespace gdk;
using namespace gdk::audio;

namespace {
    [[nodiscard]] context_unique_ptr_type make_context() {
        return openal_context::make();
    }
}

TEST_CASE("context creation", "[context]") {
    SECTION("a context can be created") {
        REQUIRE(make_context() != nullptr);
    }
}

TEST_CASE("a second live context is refused", "[context]") {
    auto pFirst = make_context();

    REQUIRE(pFirst != nullptr);
    REQUIRE_THROWS_AS(make_context(), exception);
}

TEST_CASE("the refusal is about overlap, not about counting", "[context]") {
    for (int i = 0; i < 3; ++i) {
        auto pContext = make_context();

        REQUIRE(pContext != nullptr);
    }
}

TEST_CASE("a sound describes itself", "[context][sound]") {
    auto pContext = make_context();

    SECTION("a mono clip reports one channel") {
        auto pSound = short_tone();

        REQUIRE(pSound->channel_count() == 1);
    }

    SECTION("a stereo clip reports two channels") {
        auto pSound = stereo_tone();

        REQUIRE(pSound->channel_count() == 2);
    }

    SECTION("duration matches the clips the fixtures were made at") {
        auto pShort = short_tone();
        auto pLong = long_tone();

        REQUIRE(pShort->duration() == Approx(1.0f).margin(0.001));
        REQUIRE(pLong->duration() == Approx(6.0f).margin(0.001));
    }

    SECTION("the fixtures straddle the streaming threshold") {
        auto pShort = short_tone();
        auto pLong = long_tone();

        REQUIRE(pShort->duration() < 5.0f);
        REQUIRE(pLong->duration() > 5.0f);
    }
}

TEST_CASE("a sound over already decoded audio", "[sound]") {
    SECTION("describes itself from what it was told") {
        const std::vector<unsigned char> silence(8000 * 2, 0);

        auto pSound = make_pcm_sound(silence.data(), 8000, 1, 8000, pcm_format::signed_16);

        REQUIRE(pSound->channel_count() == 1);
        REQUIRE(pSound->sample_rate() == 8000);
        REQUIRE(pSound->format() == pcm_format::signed_16);
        REQUIRE(pSound->duration() == Approx(1.0f).margin(0.001));
    }

    SECTION("eight bit is half the bytes for the same duration") {
        const std::vector<unsigned char> silence(8000, 128);

        auto pSound = make_pcm_sound(silence.data(), 8000, 1, 8000, pcm_format::unsigned_8);

        REQUIRE(pSound->duration() == Approx(1.0f).margin(0.001));
    }

    SECTION("stereo halves the duration of the same frame count at the same rate") {
        const std::vector<unsigned char> silence(8000 * 2 * 2, 0);

        auto pSound = make_pcm_sound(silence.data(), 8000, 2, 8000, pcm_format::signed_16);

        REQUIRE(pSound->channel_count() == 2);
        REQUIRE(pSound->duration() == Approx(1.0f).margin(0.001));
    }

    SECTION("a description that cannot be played is refused") {
        const std::vector<unsigned char> silence(16, 0);

        REQUIRE_THROWS_AS(make_pcm_sound(silence.data(), 8, 0, 8000, pcm_format::signed_16),
            exception);
        REQUIRE_THROWS_AS(make_pcm_sound(silence.data(), 8, 1, 0, pcm_format::signed_16),
            exception);
        REQUIRE_THROWS_AS(make_pcm_sound(nullptr, 8, 1, 8000, pcm_format::signed_16),
            exception);
    }

    SECTION("its stream reads what was put in, and rewinds") {
        std::vector<unsigned char> ramp(8 * 2);
        for (std::size_t i = 0; i < ramp.size(); ++i) ramp[i] = static_cast<unsigned char>(i);

        auto pStream = make_pcm_sound(ramp.data(), 8, 1, 8000, pcm_format::signed_16)->open();

        std::vector<unsigned char> out(8 * 2, 0xff);

        REQUIRE(pStream->read(std::as_writable_bytes(std::span(out))) == 8);
        REQUIRE(out == ramp);

        REQUIRE(pStream->read(std::as_writable_bytes(std::span(out))) == 0);

        pStream->rewind();

        REQUIRE(pStream->read(std::as_writable_bytes(std::span(out))) == 8);
    }

    SECTION("a short read reports how much it actually produced") {
        const std::vector<unsigned char> silence(4 * 2, 0);

        auto pStream = make_pcm_sound(silence.data(), 4, 1, 8000, pcm_format::signed_16)->open();

        std::vector<unsigned char> out(64, 0);

        REQUIRE(pStream->read(std::as_writable_bytes(std::span(out))) == 4);
    }
}

namespace {
    class counting_sound final : public sound {
    public:
        static constexpr std::size_t RATE = 8000;
        static constexpr std::size_t FRAMES = 1000;

        mutable std::size_t opened = 0;

        [[nodiscard]] std::size_t channel_count() const override { return 2; }
        [[nodiscard]] std::size_t sample_rate() const override { return RATE; }
        [[nodiscard]] pcm_format format() const override { return pcm_format::signed_16; }

        [[nodiscard]] audio_floating_point_type duration() const override {
            return static_cast<audio_floating_point_type>(FRAMES) / RATE;
        }

        [[nodiscard]] std::unique_ptr<pcm_stream> open() const override {
            ++opened;

            return std::unique_ptr<pcm_stream>(new ramp());
        }

    private:
        class ramp final : public pcm_stream {
            std::size_t m_Cursor = 0;

        public:
            std::size_t read(std::span<std::byte> aOut) override {
                const auto frames = std::min({aOut.size() / (2 * sizeof(std::int16_t)), FRAMES - m_Cursor, std::size_t{3}});
                auto *const out = reinterpret_cast<std::int16_t *>(aOut.data());

                for (std::size_t i = 0; i < frames; ++i) {
                    out[i * 2] = static_cast<std::int16_t>(m_Cursor + i);
                    out[i * 2 + 1] = static_cast<std::int16_t>(-static_cast<int>(m_Cursor + i));
                }

                m_Cursor += frames;

                return frames;
            }

            void rewind() override { m_Cursor = 0; }
        };
    };
}

TEST_CASE("any sound decoded now is a sound over its decoded audio", "[sound]") {
    const counting_sound original;

    const auto pDecoded = make_pcm_sound(original);

    SECTION("its layout and every frame, read through one stream") {
        REQUIRE(original.opened == 1);

        REQUIRE(pDecoded->channel_count() == 2);
        REQUIRE(pDecoded->sample_rate() == counting_sound::RATE);
        REQUIRE(pDecoded->format() == pcm_format::signed_16);
        REQUIRE(pDecoded->duration() == Approx(original.duration()));

        std::vector<std::int16_t> out(counting_sound::FRAMES * 2 + 8);

        auto pStream = pDecoded->open();

        REQUIRE(pStream->read(std::as_writable_bytes(std::span(out))) == counting_sound::FRAMES);

        for (std::size_t frame = 0; frame < counting_sound::FRAMES; ++frame) {
            INFO("frame " << frame);

            REQUIRE(out[frame * 2] == static_cast<std::int16_t>(frame));
            REQUIRE(out[frame * 2 + 1] == static_cast<std::int16_t>(-static_cast<int>(frame)));
        }
    }

    SECTION("emitters made for it never open the original again") {
        auto pContext = make_context();
        auto pScene = pContext->make_scene();

        for (int each = 0; each < 3; ++each) pScene->make_emitter(pDecoded)->play();

        REQUIRE(original.opened == 1);
    }
}

TEST_CASE("make_scene", "[context][scene]") {
    auto pContext = make_context();

    SECTION("a scene can be made") {
        REQUIRE(pContext->make_scene() != nullptr);
    }

    SECTION("scenes are distinct, and so are their listeners") {
        auto pFirst = pContext->make_scene();
        auto pSecond = pContext->make_scene();

        REQUIRE(pFirst != pSecond);
        REQUIRE(&pFirst->get_listener() != &pSecond->get_listener());
    }

    SECTION("a new scene is empty") {
        REQUIRE(pContext->make_scene()->emitter_count() == 0);
    }
}

TEST_CASE("the context policy is honoured", "[context][policy]") {
    SECTION("its defaults are usable without saying anything") {
        auto pContext = openal_context::make();

        REQUIRE(pContext != nullptr);
        REQUIRE(pContext->make_scene() != nullptr);
    }

    SECTION("the distance model reaches the device") {
        auto pContext = openal_context::make(openal_policy{distance_model::exponent_clamped});

        REQUIRE(alGetInteger(AL_DISTANCE_MODEL) == AL_EXPONENT_DISTANCE_CLAMPED);
    }

    SECTION("switching the model off reaches it too") {
        auto pContext = openal_context::make(openal_policy{distance_model::none});

        REQUIRE(alGetInteger(AL_DISTANCE_MODEL) == AL_NONE);
    }

    SECTION("doppler factor and speed of sound reach the device") {
        auto pContext = openal_context::make(
            openal_policy{.DISTANCE_MODEL = distance_model::linear_clamped,
                .DOPPLER_FACTOR = 2.5f, .SPEED_OF_SOUND = 500.0f});

        REQUIRE(alGetFloat(AL_DOPPLER_FACTOR) == Approx(2.5f));
        REQUIRE(alGetFloat(AL_SPEED_OF_SOUND) == Approx(500.0f));
    }

    SECTION("the streaming threshold decides the emitter implementation") {
        auto pContext = openal_context::make(
            openal_policy{.DISTANCE_MODEL = distance_model::linear_clamped,
                .DOPPLER_FACTOR = 1, .SPEED_OF_SOUND = 343.3f,
                .STREAMING_THRESHOLD_IN_SECONDS = 0.5f});

        auto pScene = pContext->make_scene();

        auto pEmitter = pScene->make_emitter(short_tone());

        REQUIRE(std::dynamic_pointer_cast<openal_stream_emitter>(pEmitter) != nullptr);
    }

    SECTION("a scene made from the context carries its policy") {
        auto pContext = openal_context::make(
            openal_policy{.DISTANCE_MODEL = distance_model::linear_clamped,
                .DOPPLER_FACTOR = 1, .SPEED_OF_SOUND = 343.3f,
                .STREAMING_THRESHOLD_IN_SECONDS = 100.0f});

        auto pScene = pContext->make_scene();

        auto pEmitter = pScene->make_emitter(long_tone());

        REQUIRE(std::dynamic_pointer_cast<openal_simple_emitter>(pEmitter) != nullptr);
    }
}
