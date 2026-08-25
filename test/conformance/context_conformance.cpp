// © Joseph Cameron - All Rights Reserved

#include <jfc/catch.hpp>

#include "conformance.h"

#include <gdk/audio/exception.h>
#include <gdk/audio/microphone.h>
#include <gdk/audio/scene.h>

using namespace gdk::audio;
using gdk::audio::conformance::implementation;
using gdk::audio::conformance::make_context;

namespace {
    [[nodiscard]] microphone::request small_request() {
        return microphone::request{
            .sample_rate = 8000,
            .channel_count = 1,
            .format = pcm_format::signed_16,
            .capacity_in_frames = 8000,
        };
    }
}

TEST_CASE("conformance, a context makes scenes", "[conformance][context]") {
    INFO("implementation " << implementation());

    const auto pContext = make_context();

    REQUIRE(pContext != nullptr);

    SECTION("a scene can be made") {
        REQUIRE(pContext->make_scene() != nullptr);
    }

    SECTION("scenes are distinct, and so are their listeners") {
        const auto pFirst = pContext->make_scene();
        const auto pSecond = pContext->make_scene();

        REQUIRE(pFirst != pSecond);
        REQUIRE(&pFirst->get_listener() != &pSecond->get_listener());
    }

    SECTION("a new scene is empty") {
        REQUIRE(pContext->make_scene()->emitter_count() == 0);
    }
}

TEST_CASE("conformance, capture devices are named", "[conformance][microphone]") {
    INFO("implementation " << implementation());

    const auto pContext = make_context();

    for (const auto &name : pContext->capture_device_names()) REQUIRE_FALSE(name.empty());
}

TEST_CASE("conformance, a request no device could honour is refused before anything opens", "[conformance][microphone]") {
    INFO("implementation " << implementation());

    const auto pContext = make_context();

    SECTION("no channels") {
        auto request = small_request();
        request.channel_count = 0;

        REQUIRE_THROWS_WITH(pContext->make_microphone(request), Catch::Contains("no channels"));
    }

    SECTION("no sample rate") {
        auto request = small_request();
        request.sample_rate = 0;

        REQUIRE_THROWS_WITH(pContext->make_microphone(request), Catch::Contains("no sample rate"));
    }

    SECTION("no capacity") {
        auto request = small_request();
        request.capacity_in_frames = 0;

        REQUIRE_THROWS_WITH(pContext->make_microphone(request), Catch::Contains("no capacity"));
    }

    SECTION("a device the context does not have") {
        auto request = small_request();
        request.device_name = "no such microphone";

        REQUIRE_THROWS_AS(pContext->make_microphone(request), exception);
    }
}
