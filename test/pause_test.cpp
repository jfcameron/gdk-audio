// © Joseph Cameron - All Rights Reserved

#include <jfc/catch.hpp>

#include <gdk/audio/emitter.h>
#include <gdk/audio/openal_context.h>
#include <gdk/audio/openal_policy.h>
#include <gdk/audio/scene.h>
#include <gdk/audio/sound.h>

#include "tones.h"

#include <chrono>
#include <cstddef>
#include <cstring>
#include <memory>
#include <span>
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

    class counting_sound final : public sound {
    public:
        static constexpr std::size_t RATE = 8000;

        class stream final : public pcm_stream {
        public:
            explicit stream(std::shared_ptr<std::size_t> apCursor) : m_pCursor(std::move(apCursor)) {}

            std::size_t read(std::span<std::byte> aOut) override {
                const auto frames = aOut.size() / 2;

                for (std::size_t i = 0; i < frames; ++i) {
                    const auto value = static_cast<short>(*m_pCursor + i);

                    std::memcpy(aOut.data() + i * 2, &value, 2);
                }

                *m_pCursor += frames;

                return frames;
            }

            void rewind() override {}

        private:
            std::shared_ptr<std::size_t> m_pCursor;
        };

        std::shared_ptr<std::size_t> pCursor = std::make_shared<std::size_t>(0);

        std::size_t channel_count() const override { return 1; }
        std::size_t sample_rate() const override { return RATE; }
        pcm_format format() const override { return pcm_format::signed_16; }
        audio_floating_point_type duration() const override { return 1e9f; }

        std::unique_ptr<pcm_stream> open() const override { return std::make_unique<stream>(pCursor); }
    };

    constexpr openal_policy STREAMING{
        .DISTANCE_MODEL = distance_model::linear_clamped,
        .DOPPLER_FACTOR = 1,
        .SPEED_OF_SOUND = 343.3f,
        .STREAMING_THRESHOLD_IN_SECONDS = 0,
        .STREAM_FRAMES_PER_BUFFER = 800,
        .STREAM_BUFFER_COUNT = 2,
    };
}

TEST_CASE("pausing keeps the place, and resuming carries on from it", "[pause]") {
    auto pContext = openal_context::make(STREAMING);
    auto pScene = pContext->make_scene();

    SECTION("a decoded emitter") {
        auto pEmitter = pScene->make_emitter(short_tone());

        REQUIRE_FALSE(pEmitter->is_paused());

        pEmitter->pause();
        REQUIRE_FALSE(pEmitter->is_paused());    

        pEmitter->play();
        pEmitter->pause();

        REQUIRE(pEmitter->is_paused());
        REQUIRE_FALSE(pEmitter->is_playing());

        pEmitter->resume();

        REQUIRE_FALSE(pEmitter->is_paused());
        REQUIRE(pEmitter->is_playing());
    }

    SECTION("a streaming emitter reads no further than it had while paused") {
        auto pSound = std::make_shared<counting_sound>();
        auto pEmitter = pScene->make_emitter(pSound);

        pEmitter->play();
        pump(pScene, 0.3f);

        pEmitter->pause();

        REQUIRE(pEmitter->is_paused());

        const auto readAtPause = *pSound->pCursor;

        pump(pScene, 0.3f);

        REQUIRE(*pSound->pCursor == readAtPause);
        REQUIRE(pEmitter->is_paused());

        pEmitter->resume();
        pump(pScene, 0.3f);

        REQUIRE(*pSound->pCursor > readAtPause);
        REQUIRE(pEmitter->is_playing());
    }

    SECTION("a streaming emitter that ran dry just before the pause resumes all the same") {
        auto pSound = std::make_shared<counting_sound>();
        auto pEmitter = pScene->make_emitter(pSound);

        pEmitter->play();
        pump(pScene, 0.1f);

        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        pEmitter->pause();
        pump(pScene, 0.3f);

        REQUIRE(pEmitter->is_paused());
        REQUIRE(pEmitter->playback_position());

        const auto readAtResume = *pSound->pCursor;

        pEmitter->resume();
        pump(pScene, 0.3f);

        REQUIRE(pEmitter->is_playing());
        REQUIRE(*pSound->pCursor > readAtResume);
    }

    SECTION("a pause just after a buffer was heard loses none of what is queued") {
        auto pSound = std::make_shared<counting_sound>();
        auto pEmitter = pScene->make_emitter(pSound);

        pEmitter->play();

        for (int i = 0; i < 3; ++i) {
            pump(pScene, 0.1f);
            std::this_thread::sleep_for(std::chrono::milliseconds(150));

            pEmitter->pause();
            pump(pScene, 0.1f);

            REQUIRE(pEmitter->is_paused());

            pEmitter->resume();
        }

        pump(pScene, 0.3f);

        REQUIRE(pEmitter->is_playing());
    }

    SECTION("stop() after a pause lets everything go, and play() starts again") {
        auto pEmitter = pScene->make_emitter(long_tone());

        pEmitter->play();
        pEmitter->pause();
        pEmitter->stop();

        REQUIRE_FALSE(pEmitter->is_paused());
        REQUIRE_FALSE(pEmitter->is_playing());

        pEmitter->play();

        REQUIRE(pEmitter->is_playing());
    }
}

TEST_CASE("discarding what is queued carries on from where the stream is", "[pause][discard]") {
    auto pContext = openal_context::make(STREAMING);
    auto pScene = pContext->make_scene();

    auto pSound = std::make_shared<counting_sound>();
    auto pEmitter = pScene->make_emitter(pSound);

    pEmitter->play();
    pump(pScene, 0.2f);

    *pSound->pCursor += 100000;

    const auto cursorAtDiscard = *pSound->pCursor;

    pEmitter->discard_queued();

    REQUIRE(*pSound->pCursor == cursorAtDiscard + STREAMING.STREAM_FRAMES_PER_BUFFER * STREAMING.STREAM_BUFFER_COUNT);
    REQUIRE(pEmitter->is_playing());

    pump(pScene, 0.3f);

    REQUIRE(pEmitter->is_playing());
    REQUIRE(*pSound->pCursor > cursorAtDiscard);

    SECTION("discarding while paused keeps it paused") {
        pEmitter->pause();
        pEmitter->discard_queued();

        REQUIRE(pEmitter->is_paused());
    }

    SECTION("discarding while stopped is nothing") {
        pEmitter->stop();

        const auto cursor = *pSound->pCursor;

        pEmitter->discard_queued();

        REQUIRE(*pSound->pCursor == cursor);
        REQUIRE_FALSE(pEmitter->is_playing());
    }
}
