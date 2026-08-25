// © Joseph Cameron - All Rights Reserved

#include <jfc/catch.hpp>

#include <gdk/audio/emitter.h>
#include <gdk/audio/openal_context.h>
#include <gdk/audio/openal_emitter.h>
#include <gdk/audio/openal_policy.h>
#include <gdk/audio/openal_simple_emitter.h>
#include <gdk/audio/openal_stream_emitter.h>
#include <gdk/audio/scene.h>
#include <gdk/audio/sound.h>

#include "data/tiny_tone_ogg.h"

#include <AL/al.h>

#include <chrono>
#include <memory>
#include <thread>

using namespace gdk;
using namespace gdk::audio;

namespace {
    [[nodiscard]] sound_shared_ptr_type tiny() {
        return make_vorbis_sound(tiny_tone_ogg, sizeof(tiny_tone_ogg));
    }

    [[nodiscard]] ALint al_looping_of(const emitter_shared_ptr_type &aEmitter) {
        ALint value = -1;
        alGetSourcei(std::dynamic_pointer_cast<openal_emitter>(aEmitter)->source_handle(),
            AL_LOOPING, &value);

        return value;
    }

    void pump(const scene_shared_ptr_type &aScene, const float aSeconds) {
        const auto until = std::chrono::steady_clock::now()
            + std::chrono::duration<float>(aSeconds);

        while (std::chrono::steady_clock::now() < until) {
            aScene->update();
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    }
}

TEST_CASE("looping is off until asked for", "[loop]") {
    auto pContext = openal_context::make();
    auto pScene = pContext->make_scene();

    auto pEmitter = pScene->make_emitter(tiny());

    REQUIRE_FALSE(pEmitter->is_looping());

    pEmitter->set_looping(true);
    REQUIRE(pEmitter->is_looping());

    pEmitter->set_looping(false);
    REQUIRE_FALSE(pEmitter->is_looping());
}

TEST_CASE("only the decoded emitter uses AL_LOOPING", "[loop]") {
    auto pContext = openal_context::make();
    auto pScene = pContext->make_scene();

    SECTION("a decoded emitter hands it to the source") {
        auto pEmitter = pScene->make_emitter(tiny());

        REQUIRE(std::dynamic_pointer_cast<openal_simple_emitter>(pEmitter) != nullptr);
        REQUIRE(al_looping_of(pEmitter) == AL_FALSE);

        pEmitter->set_looping(true);

        REQUIRE(al_looping_of(pEmitter) == AL_TRUE);
    }
}

TEST_CASE("a streaming emitter loops by rewinding its cursor", "[loop]") {
    auto pContext = openal_context::make(
        openal_policy{.DISTANCE_MODEL = distance_model::linear_clamped,
            .DOPPLER_FACTOR = 1, .SPEED_OF_SOUND = 343.3f,
            .STREAMING_THRESHOLD_IN_SECONDS = 0.0f});

    auto pScene = pContext->make_scene();

    auto pEmitter = pScene->make_emitter(tiny());

    REQUIRE(std::dynamic_pointer_cast<openal_stream_emitter>(pEmitter) != nullptr);

    SECTION("AL_LOOPING is left alone, since it would loop the queue") {
        pEmitter->set_looping(true);

        REQUIRE(al_looping_of(pEmitter) == AL_FALSE);
        REQUIRE(pEmitter->is_looping());
    }

    SECTION("without looping it finishes") {
        pEmitter->play();
        REQUIRE(pEmitter->is_playing());

        pump(pScene, 0.9f);

        REQUIRE_FALSE(pEmitter->is_playing());
    }

    SECTION("with looping it is still going well past the end") {
        pEmitter->set_looping(true);
        pEmitter->play();

        pump(pScene, 0.9f);

        REQUIRE(pEmitter->is_playing());
    }

    SECTION("turning looping off lets it finish") {
        pEmitter->set_looping(true);
        pEmitter->play();

        pump(pScene, 0.3f);
        REQUIRE(pEmitter->is_playing());

        pEmitter->set_looping(false);

        pump(pScene, 0.9f);

        REQUIRE_FALSE(pEmitter->is_playing());
    }
}

TEST_CASE("a decoded emitter loops too", "[loop]") {
    auto pContext = openal_context::make();
    auto pScene = pContext->make_scene();

    auto pEmitter = pScene->make_emitter(tiny());

    SECTION("without looping it finishes") {
        pEmitter->play();

        pump(pScene, 0.9f);

        REQUIRE_FALSE(pEmitter->is_playing());
    }

    SECTION("with looping it does not") {
        pEmitter->set_looping(true);
        pEmitter->play();

        pump(pScene, 0.9f);

        REQUIRE(pEmitter->is_playing());
    }
}
