// © Joseph Cameron - All Rights Reserved

#include <jfc/catch.hpp>

#include <gdk/audio/exception.h>
#include <gdk/audio/openal_context.h>
#include <gdk/audio/openal_microphone.h>

#include <chrono>
#include <thread>
#include <vector>
#include <span>

using namespace gdk::audio;

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

TEST_CASE("capture devices can be enumerated", "[microphone][openal]")
{
    auto pContext = openal_context::make();

    const auto names = pContext->capture_device_names();

    for (const auto &name : names)
    {
        INFO("device: " << name);

        REQUIRE_FALSE(name.empty());
    }
}

TEST_CASE("a request no device could honour is refused before anything opens", "[microphone][openal]")
{
    auto pContext = openal_context::make();

    SECTION("no channels")
    {
        auto r = small_request();
        r.channel_count = 0;

        REQUIRE_THROWS_AS(pContext->make_microphone(r), exception);
    }

    SECTION("no sample rate")
    {
        auto r = small_request();
        r.sample_rate = 0;

        REQUIRE_THROWS_AS(pContext->make_microphone(r), exception);
    }

    SECTION("no capacity")
    {
        auto r = small_request();
        r.capacity_in_frames = 0;

        REQUIRE_THROWS_AS(pContext->make_microphone(r), exception);
    }

    SECTION("more channels than openal captures")
    {
        auto r = small_request();
        r.channel_count = 6;

        REQUIRE_THROWS_AS(pContext->make_microphone(r), exception);
    }

    SECTION("a device that is not there")
    {
        auto r = small_request();
        r.device_name = "no such microphone";

        REQUIRE_THROWS_AS(pContext->make_microphone(r), exception);
    }
}

TEST_CASE("capturing from a real device", "[microphone][openal]")
{
    auto pContext = openal_context::make();

    if (pContext->capture_device_names().empty())
    {
        WARN("no capture device on this machine; the openal capture path was not exercised");

        return;
    }

    auto pMic = pContext->make_microphone(small_request());

    REQUIRE(pMic != nullptr);

    SECTION("it reports what was opened, not what was asked for")
    {
        REQUIRE_FALSE(pMic->device_name().empty());
        REQUIRE(pMic->sample_rate() == 8000);
        REQUIRE(pMic->channel_count() == 1);
        REQUIRE(pMic->capacity_in_frames() == 8000);
    }

    SECTION("nothing is captured before start")
    {
        pMic->update();

        REQUIRE(pMic->available_in_frames() == 0);
        REQUIRE_FALSE(pMic->capturing());
    }

    SECTION("and frames arrive once it is started and polled")
    {
        const auto began = std::chrono::steady_clock::now();

        pMic->start();

        REQUIRE(pMic->capturing());

        for (int i = 0; i < 10; ++i)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(20));

            pMic->update();
        }

        pMic->stop();

        const auto elapsed = std::chrono::duration<double>(
            std::chrono::steady_clock::now() - began).count();

        REQUIRE_FALSE(pMic->capturing());

        const auto frames = pMic->available_in_frames();
        const auto expected = elapsed * 8000.0;

        INFO("captured " << frames << " frames over " << elapsed << "s, expected about " << expected);

        REQUIRE(frames > 0);
        REQUIRE(static_cast<double>(frames) < expected * 1.5);

        SECTION("which become a sound describing them")
        {
            auto pSound = pMic->take();

            REQUIRE(pSound != nullptr);
            REQUIRE(pSound->channel_count() == 1);
            REQUIRE(pSound->sample_rate() == 8000);
            REQUIRE(pSound->duration() == Approx(static_cast<double>(frames) / 8000.0));

            REQUIRE(pMic->available_in_frames() == 0);
        }

        SECTION("or are read out in chunks")
        {
            std::vector<unsigned char> out(200);

            const auto read = pMic->read(std::as_writable_bytes(std::span(out)));

            REQUIRE(read == 100);
            REQUIRE(pMic->available_in_frames() == frames - 100);
        }
    }
}
