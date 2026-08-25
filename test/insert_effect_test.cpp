// © Joseph Cameron - All Rights Reserved

#include <jfc/catch.hpp>

#include <gdk/audio/exception.h>
#include <gdk/audio/emitter.h>
#include <gdk/audio/insert_effect.h>
#include <gdk/audio/openal_context.h>
#include <gdk/audio/openal_simple_emitter.h>
#include <gdk/audio/openal_stream_emitter.h>
#include <gdk/audio/scene.h>
#include <gdk/audio/sound.h>

#include "data/short_tone_ogg.h"

#include <algorithm>
#include <memory>
#include <vector>
#include <span>

using namespace gdk;
using namespace gdk::audio;

namespace {
    class spy_insert final : public insert_effect {
    public:
        std::size_t calls = 0;
        std::size_t frames_seen = 0;
        std::size_t resets = 0;
        pcm_format format_seen = pcm_format::unsigned_8;
        std::size_t channels_seen = 0;
        short write = 0;
        bool overwrite = false;
        std::vector<short> first_sample_in;

        void process(std::span<std::byte> aPCM, const pcm_format aFormat,
            const std::size_t aChannelCount) override {
            const auto aFrames = aPCM.size() / (aChannelCount * bytes_per_sample(aFormat));

            ++calls;
            frames_seen += aFrames;
            format_seen = aFormat;
            channels_seen = aChannelCount;

            auto *const samples = reinterpret_cast<short *>(aPCM.data());

            if (aFrames) first_sample_in.push_back(samples[0]);

            if (overwrite)
                for (std::size_t i = 0; i < aFrames * aChannelCount; ++i) samples[i] = write;
        }

        void reset() override { ++resets; }
    };

    struct fixture final {
        context_unique_ptr_type pContext = openal_context::make();
        scene_shared_ptr_type pScene = pContext->make_scene();

        [[nodiscard]] sound_shared_ptr_type sound() const {
            return make_vorbis_sound(short_tone_ogg, sizeof(short_tone_ogg));
        }
    };
}

TEST_CASE("an emitter with inserts always streams", "[insert]") {
    fixture f;

    SECTION("without inserts a short sound is decoded up front") {
        auto pEmitter = f.pScene->make_emitter(f.sound());

        REQUIRE(std::dynamic_pointer_cast<openal_simple_emitter>(pEmitter) != nullptr);
    }

    SECTION("with one it streams instead") {
        auto pEmitter = f.pScene->make_emitter(f.sound(), {std::make_shared<spy_insert>()});

        REQUIRE(std::dynamic_pointer_cast<openal_stream_emitter>(pEmitter) != nullptr);
    }

    SECTION("an empty chain is the same as no chain") {
        auto pEmitter = f.pScene->make_emitter(f.sound(), {});

        REQUIRE(std::dynamic_pointer_cast<openal_simple_emitter>(pEmitter) != nullptr);
    }
}

TEST_CASE("inserts see the audio", "[insert]") {
    fixture f;

    auto pSpy = std::make_shared<spy_insert>();
    auto pEmitter = f.pScene->make_emitter(f.sound(), {pSpy});

    SECTION("nothing is processed before playback starts") {
        REQUIRE(pSpy->calls == 0);
    }

    SECTION("playing fills the queue and runs the chain over it") {
        pEmitter->play();

        REQUIRE(pSpy->calls > 0);
        REQUIRE(pSpy->frames_seen > 0);
    }

    SECTION("it is told the layout it was handed") {
        pEmitter->play();

        REQUIRE(pSpy->format_seen == pcm_format::signed_16);
        REQUIRE(pSpy->channels_seen == 1);
    }

    SECTION("no more than a buffer at a time") {
        pEmitter->play();

        REQUIRE(pSpy->frames_seen <= pSpy->calls * 4096);
    }

    SECTION("replaying resets the chain") {
        pEmitter->play();
        pEmitter->stop();

        const auto before = pSpy->resets;

        pEmitter->play();

        REQUIRE(pSpy->resets > before);
    }
}

TEST_CASE("a chain runs in the order it was given", "[insert]") {
    fixture f;

    auto pFirst = std::make_shared<spy_insert>();
    auto pSecond = std::make_shared<spy_insert>();

    pFirst->overwrite = true;
    pFirst->write = 1234;

    auto pEmitter = f.pScene->make_emitter(f.sound(), {pFirst, pSecond});

    pEmitter->play();

    REQUIRE(pFirst->calls > 0);
    REQUIRE(pSecond->calls > 0);

    REQUIRE_FALSE(pSecond->first_sample_in.empty());
    REQUIRE(pSecond->first_sample_in.front() == 1234);
}

TEST_CASE("a null insert in the chain is refused", "[insert]") {
    fixture f;

    REQUIRE_THROWS_AS(f.pScene->make_emitter(f.sound(), {nullptr}), exception);
    REQUIRE_THROWS_AS(f.pScene->make_emitter(f.sound(),
        {std::make_shared<spy_insert>(), nullptr}), exception);
}

TEST_CASE("the shipped inserts", "[insert]") {
    fixture f;

    SECTION("a lowpass can be built and used") {
        auto pEmitter = f.pScene->make_emitter(f.sound(), {make_lowpass_insert(800.0f, 8000)});

        REQUIRE(pEmitter != nullptr);
        REQUIRE_NOTHROW(pEmitter->play());
    }

    SECTION("an echo can be built and used") {
        auto pEmitter = f.pScene->make_emitter(f.sound(),
            {make_echo_insert(0.25f, 0.4f, 0.5f, 8000, 1)});

        REQUIRE(pEmitter != nullptr);
        REQUIRE_NOTHROW(pEmitter->play());
    }

    SECTION("they refuse a sample rate they cannot work against") {
        REQUIRE_THROWS_AS(make_lowpass_insert(800.0f, 0), exception);
        REQUIRE_THROWS_AS(make_echo_insert(0.25f, 0.4f, 0.5f, 0, 1), exception);
    }

    SECTION("a lowpass leaves silence silent") {
        std::vector<short> silence(512, 0);

        make_lowpass_insert(800.0f, 8000)->process(std::as_writable_bytes(std::span(silence)),
            pcm_format::signed_16, 1);

        REQUIRE(std::all_of(silence.begin(), silence.end(), [](const short a) { return a == 0; }));
    }

    SECTION("a lowpass attenuates the fastest signal available") {
        std::vector<short> nyquist(512);

        for (std::size_t i = 0; i < nyquist.size(); ++i) nyquist[i] = i % 2 ? 20000 : -20000;

        make_lowpass_insert(200.0f, 8000)->process(std::as_writable_bytes(std::span(nyquist)),
            pcm_format::signed_16, 1);

        const auto peak = *std::max_element(nyquist.begin(), nyquist.end());

        REQUIRE(peak < 20000);
        REQUIRE(peak >= 0);
    }
}
