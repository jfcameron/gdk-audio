// © Joseph Cameron - All Rights Reserved

#include <jfc/catch.hpp>

#include <gdk/audio/emitter.h>
#include <gdk/audio/emitter_group.h>
#include <gdk/audio/exception.h>
#include <gdk/audio/null_context.h>
#include <gdk/audio/openal_context.h>
#include <gdk/audio/openal_policy.h>
#include <gdk/audio/scene.h>
#include <gdk/audio/sound.h>

#include "tones.h"

#include <array>
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

TEST_CASE("emitters in a group start, stop, pause and resume as one", "[group]") {
    auto pContext = openal_context::make(STREAMING);
    auto pScene = pContext->make_scene();

    auto pSound = long_tone();

    const std::array<emitter_shared_ptr_type, 3> members{
        pScene->make_emitter(pSound), pScene->make_emitter(pSound), pScene->make_emitter(pSound)};

    auto pGroup = pScene->make_emitter_group(std::span(members));

    REQUIRE(pGroup->size() == 3);
    REQUIRE_FALSE(pGroup->is_playing());

    pGroup->play();

    REQUIRE(pGroup->is_playing());
    for (const auto &pMember : members) REQUIRE(pMember->is_playing());

    pump(pScene, 0.2f);

    pGroup->pause();

    REQUIRE(pGroup->is_paused());
    REQUIRE_FALSE(pGroup->is_playing());
    for (const auto &pMember : members) REQUIRE(pMember->is_paused());

    pGroup->resume();

    REQUIRE(pGroup->is_playing());
    for (const auto &pMember : members) REQUIRE(pMember->is_playing());

    pump(pScene, 0.2f);

    pGroup->stop();

    REQUIRE_FALSE(pGroup->is_playing());
    for (const auto &pMember : members) REQUIRE_FALSE(pMember->is_playing());

    SECTION("a member that is let go of leaves the group") {
        auto pExtra = pScene->make_emitter(pSound);

        auto pSmall = pScene->make_emitter_group(std::array{pExtra, members[0]});

        REQUIRE(pSmall->size() == 2);

        pExtra.reset();

        REQUIRE(pSmall->size() == 1);
    }

    SECTION("an emitter of another scene is refused") {
        auto pOther = pContext->make_scene();
        const std::array<emitter_shared_ptr_type, 1> foreign{pOther->make_emitter(pSound)};

        REQUIRE_THROWS_AS(pScene->make_emitter_group(std::span(foreign)), exception);
    }
}

TEST_CASE("the null scene groups too", "[group][null]") {
    auto pContext = null_context::make();
    auto pScene = pContext->make_scene();
    auto pSound = make_pcm_sound(nullptr, 0, 1, 8000, pcm_format::signed_16);

    const std::array<emitter_shared_ptr_type, 2> members{pScene->make_emitter(pSound), pScene->make_emitter(pSound)};

    auto pGroup = pScene->make_emitter_group(std::span(members));

    pGroup->play();
    REQUIRE(pGroup->is_playing());

    pGroup->pause();
    REQUIRE(pGroup->is_paused());

    pGroup->stop();
    REQUIRE_FALSE(pGroup->is_playing());

    SECTION("and places a sound's channels, remembering where") {
        const std::array<audio_vector3_type, 2> places{{{-1, 0, 0}, {1, 0, 0}}};

        auto pPlaced = pScene->make_emitter(make_pcm_sound(nullptr, 0, 2, 8000, pcm_format::signed_16), std::span(places));

        REQUIRE(pPlaced);
        REQUIRE_FALSE(pPlaced->playback_position());

        REQUIRE_THROWS_AS(pScene->make_emitter(pSound, std::span(places)), exception);
    }
}
