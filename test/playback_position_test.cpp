// © Joseph Cameron - All Rights Reserved

#include <jfc/catch.hpp>

#include <gdk/audio/emitter.h>
#include <gdk/audio/openal_context.h>
#include <gdk/audio/openal_policy.h>
#include <gdk/audio/scene.h>
#include <gdk/audio/sound.h>

#include "tones.h"

#include <chrono>
#include <thread>

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

    constexpr openal_policy STREAMING{
        .DISTANCE_MODEL = distance_model::linear_clamped,
        .DOPPLER_FACTOR = 1,
        .SPEED_OF_SOUND = 343.3f,
        .STREAMING_THRESHOLD_IN_SECONDS = 0,
    };
}

TEST_CASE("an emitter says how far through its sound it has got", "[position]") {
    SECTION("absent until played, and again once stopped") {
        auto pContext = openal_context::make();
        auto pScene = pContext->make_scene();
        auto pEmitter = pScene->make_emitter(long_tone());

        REQUIRE_FALSE(pEmitter->playback_position());

        pEmitter->play();
        REQUIRE(pEmitter->playback_position());

        pEmitter->stop();
        REQUIRE_FALSE(pEmitter->playback_position());
    }

    for (const auto streamed : {false, true}) {
        DYNAMIC_SECTION((streamed ? "a streaming" : "a decoded") << " emitter's position follows the clock") {
            auto pContext = openal_context::make(streamed ? STREAMING : openal_policy{});
            auto pScene = pContext->make_scene();
            auto pSound = long_tone();
            auto pEmitter = pScene->make_emitter(pSound);

            const auto rate = static_cast<double>(pSound->sample_rate());

            pEmitter->play();

            const auto started = std::chrono::steady_clock::now();

            pump(pScene, 0.5f);

            const auto elapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - started).count();
            const auto position = pEmitter->playback_position();

            REQUIRE(position);
            REQUIRE(*position > 0);

            REQUIRE(*position == Approx(elapsed * rate).margin(0.1 * rate));

            pump(pScene, 0.2f);

            REQUIRE(*pEmitter->playback_position() > *position);
        }
    }

    SECTION("a pause holds it") {
        auto pContext = openal_context::make(STREAMING);
        auto pScene = pContext->make_scene();
        auto pEmitter = pScene->make_emitter(long_tone());

        pEmitter->play();
        pump(pScene, 0.3f);
        pEmitter->pause();

        const auto held = pEmitter->playback_position();

        REQUIRE(held);

        pump(pScene, 0.3f);

        REQUIRE(*pEmitter->playback_position() == Approx(*held).margin(0.01 * 48000));

        pEmitter->resume();
        pump(pScene, 0.2f);

        REQUIRE(*pEmitter->playback_position() > *held);
    }

    SECTION("a looping stream keeps counting past the sound's end") {
        auto pContext = openal_context::make(STREAMING);
        auto pScene = pContext->make_scene();
        auto pSound = short_tone();
        auto pEmitter = pScene->make_emitter(pSound);

        pEmitter->set_looping(true);
        pEmitter->play();

        const auto length = pSound->duration() * static_cast<float>(pSound->sample_rate());

        pump(pScene, pSound->duration() + 0.4f);

        REQUIRE(pEmitter->is_playing());
        REQUIRE(*pEmitter->playback_position() > length);
    }
}
