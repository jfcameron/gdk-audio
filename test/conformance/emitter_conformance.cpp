// © Joseph Cameron - MIT License

#include <jfc/catch.hpp>

#include "conformance.h"
#include "tones.h"

#include <gdk/audio/emitter.h>
#include <gdk/audio/insert_effect.h>
#include <gdk/audio/scene.h>
#include <gdk/audio/sound.h>

#include <chrono>
#include <memory>
#include <thread>

using namespace gdk::audio;
using gdk::audio::conformance::implementation;
using gdk::audio::conformance::make_context;

namespace {
    struct fixture final {
        context_shared_ptr_type pContext = make_context();
        scene_shared_ptr_type pScene = pContext->make_scene();
    };
}

TEST_CASE("conformance, an emitter plays and stops when told", "[conformance][emitter]") {
    INFO("implementation " << implementation());

    fixture f;

    for (const auto &pSound : {short_tone(), long_tone()}) {
        INFO(pSound->duration() << " seconds");

        const auto pEmitter = f.pScene->make_emitter(pSound);

        REQUIRE_FALSE(pEmitter->is_playing());
        REQUIRE_FALSE(pEmitter->is_paused());

        REQUIRE_NOTHROW(pEmitter->stop());
        REQUIRE_FALSE(pEmitter->is_playing());

        pEmitter->play();
        pEmitter->play();
        REQUIRE(pEmitter->is_playing());

        pEmitter->stop();
        REQUIRE_FALSE(pEmitter->is_playing());

        pEmitter->play();
        REQUIRE(pEmitter->is_playing());
    }
}

TEST_CASE("conformance, a pause holds an emitter until it is resumed", "[conformance][emitter][pause]") {
    INFO("implementation " << implementation());

    fixture f;

    const auto pEmitter = f.pScene->make_emitter(short_tone());

    SECTION("an emitter that is not playing does not pause") {
        pEmitter->pause();

        REQUIRE_FALSE(pEmitter->is_paused());
        REQUIRE_FALSE(pEmitter->is_playing());
    }

    SECTION("paused, it is not playing, and resumed, it is again") {
        pEmitter->play();
        pEmitter->pause();

        REQUIRE(pEmitter->is_paused());
        REQUIRE_FALSE(pEmitter->is_playing());

        pEmitter->resume();

        REQUIRE_FALSE(pEmitter->is_paused());
        REQUIRE(pEmitter->is_playing());
    }

    SECTION("stopped after a pause, it is neither") {
        pEmitter->play();
        pEmitter->pause();
        pEmitter->stop();

        REQUIRE_FALSE(pEmitter->is_paused());
        REQUIRE_FALSE(pEmitter->is_playing());
    }
}

TEST_CASE("conformance, an emitter keeps what it is set to", "[conformance][emitter]") {
    INFO("implementation " << implementation());

    fixture f;

    const auto pEmitter = f.pScene->make_emitter(short_tone());

    SECTION("position, velocity and direction") {
        pEmitter->set_position({1.5f, -2.5f, 3.5f});
        pEmitter->set_velocity({0, 0, -4});
        pEmitter->set_direction({0, 0, 1});

        REQUIRE(pEmitter->position() == audio_vector3_type(1.5f, -2.5f, 3.5f));
        REQUIRE(pEmitter->velocity() == audio_vector3_type(0, 0, -4));
        REQUIRE(pEmitter->direction() == audio_vector3_type(0, 0, 1));
    }

    SECTION("pitch and gain, whole until told otherwise") {
        REQUIRE(pEmitter->pitch() == Approx(1.0f));
        REQUIRE(pEmitter->gain() == Approx(1.0f));

        pEmitter->set_pitch(0.5f);
        pEmitter->set_gain(0.25f);

        REQUIRE(pEmitter->pitch() == Approx(0.5f));
        REQUIRE(pEmitter->gain() == Approx(0.25f));
    }

    SECTION("and takes it while playing") {
        pEmitter->play();

        pEmitter->set_pitch(1.25f);
        pEmitter->set_position({1, 2, 3});

        REQUIRE(pEmitter->pitch() == Approx(1.25f));
        REQUIRE(pEmitter->position() == audio_vector3_type(1, 2, 3));
        REQUIRE(pEmitter->is_playing());
    }

    SECTION("a cone and an attenuation are accepted") {
        REQUIRE_NOTHROW(pEmitter->set_cone(1.0f, 2.0f, 0.5f));
        REQUIRE_NOTHROW(pEmitter->set_attenuation(1.0f, 10.0f, 1.0f));
        REQUIRE_NOTHROW(pEmitter->set_attenuation(1.0f, 10.0f, 0.0f));
    }
}

TEST_CASE("conformance, looping is off until asked for", "[conformance][emitter][loop]") {
    INFO("implementation " << implementation());

    fixture f;

    const auto pEmitter = f.pScene->make_emitter(tiny_tone());

    REQUIRE_FALSE(pEmitter->is_looping());

    pEmitter->set_looping(true);
    REQUIRE(pEmitter->is_looping());

    pEmitter->set_looping(false);
    REQUIRE_FALSE(pEmitter->is_looping());
}

TEST_CASE("conformance, a stopped emitter has no playback position", "[conformance][emitter][position]") {
    INFO("implementation " << implementation());

    fixture f;

    const auto pEmitter = f.pScene->make_emitter(long_tone());

    REQUIRE_FALSE(pEmitter->playback_position());

    pEmitter->play();
    pEmitter->stop();

    REQUIRE_FALSE(pEmitter->playback_position());
}

TEST_CASE("conformance, an emitter outlives the sound it was made of", "[conformance][emitter][sound]") {
    INFO("implementation " << implementation());

    fixture f;

    emitter_shared_ptr_type pEmitter;

    {
        const auto pSound = short_tone();
        pEmitter = f.pScene->make_emitter(pSound);
    }

    REQUIRE(pEmitter != nullptr);
    REQUIRE_NOTHROW(pEmitter->play());
    REQUIRE(pEmitter->is_playing());
}

TEST_CASE("conformance, an emitter plays through the shipped inserts", "[conformance][emitter][insert]") {
    INFO("implementation " << implementation());

    fixture f;

    for (const auto &pInsert : {make_lowpass_insert(800.0f, 8000), make_echo_insert(0.25f, 0.4f, 0.5f, 8000, 1)}) {
        const auto pEmitter = f.pScene->make_emitter(short_tone(), {pInsert});

        REQUIRE(pEmitter != nullptr);

        pEmitter->play();

        REQUIRE(pEmitter->is_playing());
    }
}

namespace {
    void run_for(scene &aScene, const double aSeconds) {
        const auto end = std::chrono::steady_clock::now() + std::chrono::duration<double>(aSeconds);

        while (std::chrono::steady_clock::now() < end) {
            aScene.update();

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
}

TEST_CASE("conformance, play() on a playing emitter starts its sound again", "[conformance][emitter][position]") {
    INFO("implementation " << implementation());

    fixture f;

    for (const auto &pSound : {short_tone(), long_tone()}) {
        INFO("a sound of " << pSound->duration() << " seconds");

        const auto pEmitter = f.pScene->make_emitter(pSound);

        pEmitter->play();
        run_for(*f.pScene, 0.4);

        const auto before = pEmitter->playback_position();

        pEmitter->play();
        f.pScene->update();

        REQUIRE(pEmitter->is_playing());

        if (const auto after = pEmitter->playback_position(); before && after)
            REQUIRE(*after < *before);
    }
}

TEST_CASE("conformance, a looping emitter's position counts past the sound's end", "[conformance][emitter][position][loop]") {
    INFO("implementation " << implementation());

    fixture f;

    const auto pSound = tiny_tone();
    const auto pEmitter = f.pScene->make_emitter(pSound);

    pEmitter->set_looping(true);
    pEmitter->play();
    run_for(*f.pScene, 0.6);

    REQUIRE(pEmitter->is_playing());

    if (const auto position = pEmitter->playback_position())
        REQUIRE(*position > pSound->duration() * static_cast<double>(pSound->sample_rate()));
}
