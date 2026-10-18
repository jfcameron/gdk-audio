// © Joseph Cameron - All Rights Reserved

#include <jfc/catch.hpp>

#include <gdk/audio/context.h>
#include <gdk/audio/openal_context.h>
#include <gdk/audio/scene.h>
#include <gdk/audio/emitter.h>
#include <gdk/audio/sound.h>

#include <gdk/audio/openal_simple_emitter.h>
#include <gdk/audio/openal_stream_emitter.h>

#include "tones.h"

#include <limits>
#include <memory>

using namespace gdk;
using namespace gdk::audio;

namespace {
    struct fixture final {
        context_unique_ptr_type pContext = openal_context::make();
        scene_shared_ptr_type pScene = pContext->make_scene();

        [[nodiscard]] std::shared_ptr<emitter> emitter_for(const sound_shared_ptr_type &apSound) {
            return pScene->make_emitter(apSound);
        }
    };
}

TEST_CASE("clip length selects the emitter implementation", "[emitter]") {
    fixture f;

    SECTION("a clip under five seconds is fully decoded") {
        auto pEmitter = f.emitter_for(short_tone());

        REQUIRE(std::dynamic_pointer_cast<openal_simple_emitter>(pEmitter) != nullptr);
        REQUIRE(std::dynamic_pointer_cast<openal_stream_emitter>(pEmitter) == nullptr);
    }

    SECTION("a clip over five seconds is streamed") {
        auto pEmitter = f.emitter_for(long_tone());

        REQUIRE(std::dynamic_pointer_cast<openal_stream_emitter>(pEmitter) != nullptr);
        REQUIRE(std::dynamic_pointer_cast<openal_simple_emitter>(pEmitter) == nullptr);
    }
}

TEST_CASE("playback state", "[emitter]") {
    fixture f;

    SECTION("a new emitter is not playing") {
        REQUIRE_FALSE(f.emitter_for(short_tone())->is_playing());
    }

    SECTION("play starts it and stop ends it") {
        auto pEmitter = f.emitter_for(short_tone());

        pEmitter->play();
        REQUIRE(pEmitter->is_playing());

        pEmitter->stop();
        REQUIRE_FALSE(pEmitter->is_playing());
    }

    SECTION("stopping an emitter that never played is harmless") {
        auto pEmitter = f.emitter_for(short_tone());

        REQUIRE_NOTHROW(pEmitter->stop());
        REQUIRE_FALSE(pEmitter->is_playing());
    }

    SECTION("play is idempotent") {
        auto pEmitter = f.emitter_for(short_tone());

        pEmitter->play();
        pEmitter->play();

        REQUIRE(pEmitter->is_playing());
    }

    SECTION("a stopped emitter can play again") {
        auto pEmitter = f.emitter_for(short_tone());

        pEmitter->play();
        pEmitter->stop();
        pEmitter->play();

        REQUIRE(pEmitter->is_playing());
    }

    SECTION("a streaming emitter reports the same states") {
        auto pEmitter = f.emitter_for(long_tone());

        REQUIRE_FALSE(pEmitter->is_playing());

        pEmitter->play();
        REQUIRE(pEmitter->is_playing());

        pEmitter->stop();
        REQUIRE_FALSE(pEmitter->is_playing());
    }
}

TEST_CASE("emitter parameters are accepted", "[emitter]") {
    fixture f;

    auto pEmitter = f.emitter_for(short_tone());

    SECTION("position") {
        REQUIRE_NOTHROW(pEmitter->set_position({0, 0, 0}));
        REQUIRE_NOTHROW(pEmitter->set_position({1.5f, -2.5f, 3.5f}));
        REQUIRE_NOTHROW(pEmitter->set_position({-1e6f, 1e6f, 0}));
    }

    SECTION("pitch") {
        REQUIRE_NOTHROW(pEmitter->set_pitch(1.0f));
        REQUIRE_NOTHROW(pEmitter->set_pitch(0.5f));
        REQUIRE_NOTHROW(pEmitter->set_pitch(2.0f));
    }

    SECTION("parameters can be set while playing") {
        pEmitter->play();

        REQUIRE_NOTHROW(pEmitter->set_pitch(1.25f));
        REQUIRE_NOTHROW(pEmitter->set_position({1, 2, 3}));
        REQUIRE(pEmitter->is_playing());
    }
}

TEST_CASE("an emitter outlives the sound it was built from", "[emitter][sound]") {
    fixture f;

    std::shared_ptr<emitter> pEmitter;

    {
        auto pSound = short_tone();
        pEmitter = f.pScene->make_emitter(pSound);
    }

    REQUIRE(pEmitter != nullptr);
    REQUIRE_NOTHROW(pEmitter->play());
    REQUIRE(pEmitter->is_playing());
}
