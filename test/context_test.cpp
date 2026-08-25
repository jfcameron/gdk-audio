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

#include "data/long_tone_ogg.h"
#include "data/short_tone_ogg.h"
#include "data/stereo_tone_ogg.h"

#include <AL/al.h>

#include <memory>
#include <vector>
#include <span>

using namespace gdk;
using namespace gdk::audio;

namespace {
    [[nodiscard]] sound_shared_ptr_type sound_from(const unsigned char *const aData,
        const std::size_t aSize) {
        return make_vorbis_sound(aData, aSize);
    }

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

TEST_CASE("make_sound", "[context][sound]") {
    auto pContext = make_context();

    SECTION("a well formed vorbis buffer yields a sound") {
        REQUIRE(sound_from(short_tone_ogg, sizeof(short_tone_ogg)) != nullptr);
    }

    SECTION("a malformed buffer is refused") {
        const std::vector<unsigned char> garbage(64, 0x7f);

        REQUIRE_THROWS_AS(sound_from(garbage.data(), garbage.size()), exception);
    }

    SECTION("an empty buffer is refused") {
        REQUIRE_THROWS_AS(sound_from(short_tone_ogg, 0), exception);
    }

    SECTION("a null pointer is refused rather than dereferenced") {
        REQUIRE_THROWS_AS(sound_from(nullptr, 0), exception);
    }

    SECTION("a null pointer with a nonzero length is refused too") {
        REQUIRE_THROWS_AS(sound_from(nullptr, 1024), exception);
    }

    SECTION("a length shorter than the data truncates rather than overruns") {
        REQUIRE_THROWS_AS(sound_from(short_tone_ogg, 8), exception);
    }

    SECTION("truncated vorbis is refused") {
        REQUIRE_THROWS_AS(sound_from(short_tone_ogg, sizeof(short_tone_ogg) / 4),
            exception);
    }
}

TEST_CASE("a sound describes itself", "[context][sound]") {
    auto pContext = make_context();

    SECTION("a mono clip reports one channel") {
        auto pSound = sound_from(short_tone_ogg, sizeof(short_tone_ogg));

        REQUIRE(pSound->channel_count() == 1);
    }

    SECTION("a stereo clip reports two channels") {
        auto pSound = sound_from(stereo_tone_ogg, sizeof(stereo_tone_ogg));

        REQUIRE(pSound->channel_count() == 2);
    }

    SECTION("duration matches the clips the fixtures were generated at") {
        auto pShort = sound_from(short_tone_ogg, sizeof(short_tone_ogg));
        auto pLong = sound_from(long_tone_ogg, sizeof(long_tone_ogg));

        REQUIRE(pShort->duration() == Approx(1.0f).margin(0.05));
        REQUIRE(pLong->duration() == Approx(6.0f).margin(0.05));
    }

    SECTION("the fixtures straddle the streaming threshold") {
        auto pShort = sound_from(short_tone_ogg, sizeof(short_tone_ogg));
        auto pLong = sound_from(long_tone_ogg, sizeof(long_tone_ogg));

        REQUIRE(pShort->duration() < 5.0f);
        REQUIRE(pLong->duration() > 5.0f);
    }

    SECTION("vorbis reports the layout it decodes to") {
        auto pSound = sound_from(short_tone_ogg, sizeof(short_tone_ogg));

        REQUIRE(pSound->format() == pcm_format::signed_16);
        REQUIRE(pSound->sample_rate() == 8000);
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

        auto pEmitter = pScene->make_emitter(sound_from(short_tone_ogg, sizeof(short_tone_ogg)));

        REQUIRE(std::dynamic_pointer_cast<openal_stream_emitter>(pEmitter) != nullptr);
    }

    SECTION("a scene made from the context carries its policy") {
        auto pContext = openal_context::make(
            openal_policy{.DISTANCE_MODEL = distance_model::linear_clamped,
                .DOPPLER_FACTOR = 1, .SPEED_OF_SOUND = 343.3f,
                .STREAMING_THRESHOLD_IN_SECONDS = 100.0f});

        auto pScene = pContext->make_scene();

        auto pEmitter = pScene->make_emitter(sound_from(long_tone_ogg, sizeof(long_tone_ogg)));

        REQUIRE(std::dynamic_pointer_cast<openal_simple_emitter>(pEmitter) != nullptr);
    }
}
