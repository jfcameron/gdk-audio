// © Joseph Cameron - All Rights Reserved

#include <jfc/catch.hpp>

#include <gdk/audio/emitter.h>
#include <gdk/audio/exception.h>
#include <gdk/audio/openal_context.h>
#include <gdk/audio/openal_emitter.h>
#include <gdk/audio/openal_policy.h>
#include <gdk/audio/openal_stream_emitter.h>
#include <gdk/audio/scene.h>
#include <gdk/audio/sound.h>

#include <AL/al.h>

#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <thread>
#include <vector>

using namespace gdk;
using namespace gdk::audio;

namespace {
    void pump(const scene_shared_ptr_type &aScene, const float aSeconds) {
        const auto until = std::chrono::steady_clock::now() + std::chrono::duration<float>(aSeconds);

        while (std::chrono::steady_clock::now() < until) {
            aScene->update();
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    }

    constexpr std::size_t RATE = 8000;

    [[nodiscard]] sound_shared_ptr_type tones(const std::size_t aChannels, const pcm_format aFormat) {
        const std::size_t frames = RATE;

        if (aFormat == pcm_format::float_32) {
            std::vector<float> pcm(frames * aChannels);

            for (std::size_t i = 0; i < frames; ++i)
                for (std::size_t c = 0; c < aChannels; ++c)
                    pcm[i * aChannels + c] = std::sin(static_cast<float>(i) * (220.f + 110.f * c) * 6.2832f / RATE);

            return make_pcm_sound(pcm.data(), frames, aChannels, RATE, aFormat);
        }

        std::vector<std::int16_t> pcm(frames * aChannels);

        for (std::size_t i = 0; i < frames; ++i)
            for (std::size_t c = 0; c < aChannels; ++c)
                pcm[i * aChannels + c] = static_cast<std::int16_t>(
                    10000 * std::sin(static_cast<float>(i) * (220.f + 110.f * c) * 6.2832f / RATE));

        return make_pcm_sound(pcm.data(), frames, aChannels, RATE, aFormat);
    }

    constexpr float D = 0.7071f;

    const std::array<audio_vector3_type, 6> SURROUND{{
        {-D, 0, -D}, {D, 0, -D}, {0, 0, -1}, {0, -0.5f, -0.866f}, {-D, 0, D}, {D, 0, D}}};

    [[nodiscard]] ALint al_state_of(const ALuint aSource) {
        ALint state = AL_STOPPED;
        alGetSourcei(aSource, AL_SOURCE_STATE, &state);

        return state;
    }
}

TEST_CASE("a multichannel sound played a channel from each place", "[multichannel]") {
    auto pContext = openal_context::make();
    auto pScene = pContext->make_scene();

    SECTION("has a source a channel, and every one of them plays, pauses and stops together") {
        auto pEmitter = pScene->make_emitter(tones(6, pcm_format::signed_16), std::span(SURROUND));

        const auto pSources = std::dynamic_pointer_cast<openal_emitter>(pEmitter);

        REQUIRE(pSources);
        REQUIRE(pSources->source_handles().size() == 6);

        REQUIRE(std::dynamic_pointer_cast<openal_stream_emitter>(pEmitter));

        pEmitter->play();

        for (const auto source : pSources->source_handles()) REQUIRE(al_state_of(source) == AL_PLAYING);

        pump(pScene, 0.2f);

        pEmitter->pause();

        for (const auto source : pSources->source_handles()) REQUIRE(al_state_of(source) == AL_PAUSED);

        pEmitter->resume();

        for (const auto source : pSources->source_handles()) REQUIRE(al_state_of(source) == AL_PLAYING);

        pEmitter->stop();

        for (const auto source : pSources->source_handles()) REQUIRE(al_state_of(source) != AL_PLAYING);

        REQUIRE(alGetError() == AL_NO_ERROR);
    }

    SECTION("plays through to the end, and says one position for the whole sound") {
        auto pSound = tones(2, pcm_format::signed_16);
        auto pEmitter = pScene->make_emitter(pSound, std::span(SURROUND).first(2));

        pEmitter->play();
        pump(pScene, 0.5f);

        REQUIRE(pEmitter->is_playing());

        const auto position = pEmitter->playback_position();

        REQUIRE(position);
        REQUIRE(*position == Approx(0.5 * RATE).margin(0.1 * RATE));

        pump(pScene, 0.8f);

        REQUIRE_FALSE(pEmitter->is_playing());
    }

    SECTION("each channel stands where it was put, about the emitter") {
        auto pEmitter = pScene->make_emitter(tones(2, pcm_format::signed_16), std::span(SURROUND).first(2));

        pEmitter->set_position({10, 0, 0});
        pScene->update();

        const auto sources = std::dynamic_pointer_cast<openal_emitter>(pEmitter)->source_handles();

        std::array<ALfloat, 3> left{}, right{};
        alGetSourcefv(sources[0], AL_POSITION, left.data());
        alGetSourcefv(sources[1], AL_POSITION, right.data());

        REQUIRE(left[0] == Approx(10 - D));
        REQUIRE(right[0] == Approx(10 + D));
        REQUIRE(left[2] == Approx(-D));
    }

    SECTION("needs a place for every channel, and no more") {
        REQUIRE_THROWS_AS(pScene->make_emitter(tones(6, pcm_format::signed_16), std::span(SURROUND).first(2)), exception);
        REQUIRE_THROWS_AS(pScene->make_emitter(tones(1, pcm_format::signed_16), std::span(SURROUND)), exception);
        REQUIRE_THROWS_AS(pScene->make_emitter(tones(1, pcm_format::signed_16), std::span<const audio_vector3_type>()), exception);
    }
}

TEST_CASE("a multichannel sound as a speaker bed", "[multichannel]") {
    auto pContext = openal_context::make();
    auto pScene = pContext->make_scene();

    if (!alIsExtensionPresent("AL_EXT_MCFORMATS")) {
        REQUIRE_THROWS_AS(pScene->make_emitter(tones(6, pcm_format::signed_16)), exception);

        return;
    }

    auto pEmitter = pScene->make_emitter(tones(6, pcm_format::signed_16));

    REQUIRE(std::dynamic_pointer_cast<openal_emitter>(pEmitter)->source_handles().size() == 1);

    pEmitter->play();

    REQUIRE(pEmitter->is_playing());
    REQUIRE(alGetError() == AL_NO_ERROR);
}

TEST_CASE("float samples", "[multichannel][float]") {
    auto pContext = openal_context::make();
    auto pScene = pContext->make_scene();

    REQUIRE(bytes_per_sample(pcm_format::float_32) == 4);

    if (!alIsExtensionPresent("AL_EXT_float32")) {
        REQUIRE_THROWS_AS(pScene->make_emitter(tones(1, pcm_format::float_32)), exception);

        return;
    }

    SECTION("play as decoded, and placed a channel at a time") {
        for (const auto &pEmitter : {pScene->make_emitter(tones(2, pcm_format::float_32)),
            pScene->make_emitter(tones(2, pcm_format::float_32), std::span(SURROUND).first(2))}) {
            pEmitter->play();

            REQUIRE(pEmitter->is_playing());
            REQUIRE(alGetError() == AL_NO_ERROR);
        }
    }

    SECTION("pass through an insert effect") {
        auto pEmitter = pScene->make_emitter(tones(1, pcm_format::float_32), {make_lowpass_insert(1000, RATE)});

        pEmitter->play();
        pump(pScene, 0.1f);

        REQUIRE(pEmitter->is_playing());
        REQUIRE(alGetError() == AL_NO_ERROR);
    }
}
