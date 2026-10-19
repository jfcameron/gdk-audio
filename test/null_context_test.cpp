// © Joseph Cameron - All Rights Reserved

#include <jfc/catch.hpp>

#include <gdk/audio/exception.h>
#include <gdk/audio/null_context.h>

#include <cstring>
#include <vector>
#include <span>

using namespace gdk::audio;

namespace {
    [[nodiscard]] microphone::request mono16(const std::size_t aCapacityFrames,
        const bool aDropOldest = false) {
        return microphone::request{
            .sample_rate = 8000,
            .channel_count = 1,
            .format = pcm_format::signed_16,
            .capacity_in_frames = aCapacityFrames,
            .drop_oldest_when_full = aDropOldest,
        };
    }

    [[nodiscard]] std::vector<unsigned char> ramp(const std::size_t aFrameCount,
        const unsigned char aFirst = 0) {
        std::vector<unsigned char> out(aFrameCount * 2);

        for (std::size_t i = 0; i < out.size(); ++i) out[i] = static_cast<unsigned char>(aFirst + i);

        return out;
    }
}

TEST_CASE("a context with no hardware still builds a scene", "[null]")
{
    auto pContext = null_context::make();

    REQUIRE(pContext != nullptr);

    auto pScene = pContext->make_scene();

    REQUIRE(pScene != nullptr);
    REQUIRE(pContext->scene_count() == 1);

    SECTION("whose listener remembers what it was told")
    {
        auto &listener = pScene->get_listener();

        listener.set_position({1, 2, 3});
        listener.set_gain(0.25f);

        REQUIRE(listener.position() == audio_vector3_type{1, 2, 3});
        REQUIRE(listener.gain() == Approx(0.25f));
    }

    SECTION("and whose emitters are held weakly, as the real one holds them")
    {
        auto pSound = make_pcm_sound(nullptr, 0, 1, 8000, pcm_format::signed_16);

        {
            auto pEmitter = pScene->make_emitter(pSound);

            REQUIRE(pScene->emitter_count() == 1);
        }

        REQUIRE(pScene->emitter_count() == 0);
    }
}

TEST_CASE("a microphone captures only between start and stop", "[null][microphone]")
{
    auto pContext = null_context::make();

    auto pMic = pContext->make_microphone(mono16(1000));
    auto pDriver = std::dynamic_pointer_cast<null_microphone>(pMic);

    REQUIRE(pDriver != nullptr);
    REQUIRE_FALSE(pMic->capturing());

    const auto data = ramp(10);

    SECTION("nothing arrives before start")
    {
        pDriver->offer(data.data(), data.size());
        pMic->update();

        REQUIRE(pMic->available_in_frames() == 0);
    }

    SECTION("and it does after")
    {
        pMic->start();

        REQUIRE(pMic->capturing());

        pDriver->offer(data.data(), data.size());

        REQUIRE(pMic->available_in_frames() == 0);

        pMic->update();

        REQUIRE(pMic->available_in_frames() == 10);
    }

    SECTION("stopping keeps what was collected and drops what was not")
    {
        pMic->start();
        pDriver->offer(data.data(), data.size());
        pMic->update();

        pDriver->offer(data.data(), data.size());
        pMic->stop();

        REQUIRE_FALSE(pMic->capturing());
        REQUIRE(pMic->available_in_frames() == 10);

        pMic->update();

        REQUIRE(pMic->available_in_frames() == 10);
    }
}

TEST_CASE("the single buffer mode keeps the beginning", "[null][microphone]")
{
    auto pContext = null_context::make();

    auto pMic = pContext->make_microphone(mono16(8));
    auto pDriver = std::dynamic_pointer_cast<null_microphone>(pMic);

    pMic->start();

    const auto first = ramp(8, 1);
    const auto second = ramp(8, 100);

    pDriver->offer(first.data(), first.size());
    pMic->update();

    REQUIRE(pMic->capacity_reached());
    REQUIRE(pMic->capturing());

    pDriver->offer(second.data(), second.size());
    pMic->update();

    REQUIRE_FALSE(pMic->capturing());
    REQUIRE(pMic->available_in_frames() == 8);

    REQUIRE(pMic->dropped_frame_count() == 8);

    SECTION("and what it kept is the beginning, not the end")
    {
        std::vector<unsigned char> out(16);

        REQUIRE(pMic->read(std::as_writable_bytes(std::span(out))) == 8);
        REQUIRE(std::memcmp(out.data(), first.data(), first.size()) == 0);
    }
}

TEST_CASE("the streaming mode keeps the newest", "[null][microphone]")
{
    auto pContext = null_context::make();

    auto pMic = pContext->make_microphone(mono16(8, true));
    auto pDriver = std::dynamic_pointer_cast<null_microphone>(pMic);

    pMic->start();

    const auto first = ramp(8, 1);
    const auto second = ramp(8, 100);

    pDriver->offer(first.data(), first.size());
    pMic->update();
    pDriver->offer(second.data(), second.size());
    pMic->update();

    REQUIRE(pMic->capturing());
    REQUIRE(pMic->available_in_frames() == 8);

    REQUIRE(pMic->dropped_frame_count() == 8);

    std::vector<unsigned char> out(16);

    REQUIRE(pMic->read(std::as_writable_bytes(std::span(out))) == 8);
    REQUIRE(std::memcmp(out.data(), second.data(), second.size()) == 0);
}

TEST_CASE("reading takes frames out, taking empties the buffer", "[null][microphone]")
{
    auto pContext = null_context::make();

    auto pMic = pContext->make_microphone(mono16(1000));
    auto pDriver = std::dynamic_pointer_cast<null_microphone>(pMic);

    pMic->start();

    const auto data = ramp(10);

    pDriver->offer(data.data(), data.size());
    pMic->update();

    SECTION("read gives what was asked for, or what there is")
    {
        std::vector<unsigned char> out(40);

        REQUIRE(pMic->read(std::as_writable_bytes(std::span(out)).first(4 * 2)) == 4);
        REQUIRE(pMic->available_in_frames() == 6);

        REQUIRE(pMic->read(std::as_writable_bytes(std::span(out))) == 6);
        REQUIRE(pMic->available_in_frames() == 0);

        REQUIRE(pMic->read(std::as_writable_bytes(std::span(out)).first(4 * 2)) == 0);
    }

    SECTION("take hands over a sound describing what was captured")
    {
        auto pSound = pMic->take();

        REQUIRE(pSound != nullptr);
        REQUIRE(pSound->channel_count() == 1);
        REQUIRE(pSound->sample_rate() == 8000);
        REQUIRE(pSound->format() == pcm_format::signed_16);
        REQUIRE(pSound->duration() == Approx(10.0 / 8000.0));

        REQUIRE(pMic->available_in_frames() == 0);
    }

    SECTION("taking a capture nobody spoke during is a sound with no frames, not nothing")
    {
        pMic->discard();

        auto pSound = pMic->take();

        REQUIRE(pSound != nullptr);
        REQUIRE(pSound->duration() == Approx(0.0));
    }
}

TEST_CASE("permission can be refused after a successful start", "[null][microphone]")
{
    auto pContext = null_context::make();

    auto pMic = pContext->make_microphone(mono16(1000));
    auto pDriver = std::dynamic_pointer_cast<null_microphone>(pMic);

    pMic->start();

    REQUIRE(pMic->capturing());

    pDriver->revoke();

    REQUIRE_FALSE(pMic->capturing());
    REQUIRE(pMic->available_in_frames() == 0);
}

TEST_CASE("a device must be one the context offers", "[null][microphone]")
{
    auto pContext = null_context::make();

    REQUIRE(pContext->capture_device_names().empty());

    auto named = mono16(100);
    named.device_name = "Some USB Microphone";

    REQUIRE_THROWS_AS(pContext->make_microphone(named), exception);

    pContext->set_capture_device_names({"Some USB Microphone", "Built-in"});

    REQUIRE(pContext->capture_device_names().size() == 2);
    REQUIRE(pContext->make_microphone(named) != nullptr);

    SECTION("and the default always opens, since a fake has no hardware to enumerate")
    {
        auto pDefault = pContext->make_microphone(mono16(100));

        REQUIRE(pDefault != nullptr);
        REQUIRE_FALSE(pDefault->device_name().empty());
    }
}

TEST_CASE("a microphone reports what it opened, not what was asked for", "[null][microphone]")
{
    auto pContext = null_context::make();

    auto pMic = pContext->make_microphone(mono16(500));

    REQUIRE(pMic->sample_rate() == 8000);
    REQUIRE(pMic->channel_count() == 1);
    REQUIRE(pMic->format() == pcm_format::signed_16);
    REQUIRE(pMic->capacity_in_frames() == 500);

    SECTION("and a request it cannot honour at all is refused rather than silently altered")
    {
        auto broken = mono16(500);
        broken.channel_count = 0;

        REQUIRE_THROWS_AS(pContext->make_microphone(broken), exception);

        broken = mono16(0);

        REQUIRE_THROWS_AS(pContext->make_microphone(broken), exception);
    }
}
