// © Joseph Cameron - All Rights Reserved

#include <jfc/catch.hpp>

#include <gdk/audio/exception.h>
#include <gdk/audio/emitter.h>
#include <gdk/audio/ext/vorbis.h>
#include <gdk/audio/openal_context.h>
#include <gdk/audio/openal_simple_emitter.h>
#include <gdk/audio/openal_stream_emitter.h>
#include <gdk/audio/scene.h>
#include <gdk/audio/sound.h>

#include "data/long_tone_ogg.h"
#include "data/short_tone_ogg.h"
#include "data/stereo_tone_ogg.h"

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>
#include <vector>

using namespace gdk::audio;

namespace {
    template <std::size_t size_type>
    [[nodiscard]] std::vector<std::byte> bytes_of(const unsigned char (&aData)[size_type], const std::size_t aSize = size_type) {
        const auto *const from = reinterpret_cast<const std::byte *>(aData);

        return {from, from + aSize};
    }
}

TEST_CASE("ogg vorbis is decoded, and anything else refused", "[ext][vorbis]") {
    SECTION("a well formed vorbis buffer yields a sound") {
        REQUIRE(ext::make_vorbis_sound(bytes_of(short_tone_ogg)) != nullptr);
    }

    SECTION("a malformed buffer is refused") {
        REQUIRE_THROWS_AS(ext::make_vorbis_sound(std::vector<std::byte>(64, std::byte{0x7f})), exception);
    }

    SECTION("an empty buffer is refused") {
        REQUIRE_THROWS_AS(ext::make_vorbis_sound({}), exception);
    }

    SECTION("a buffer cut short after its header is refused") {
        REQUIRE_THROWS_AS(ext::make_vorbis_sound(bytes_of(short_tone_ogg, 8)), exception);
    }

    SECTION("truncated vorbis is refused") {
        REQUIRE_THROWS_AS(ext::make_vorbis_sound(bytes_of(short_tone_ogg, sizeof(short_tone_ogg) / 4)), exception);
    }
}

TEST_CASE("a vorbis sound describes itself", "[ext][vorbis]") {
    SECTION("mono and stereo, as encoded") {
        REQUIRE(ext::make_vorbis_sound(bytes_of(short_tone_ogg))->channel_count() == 1);
        REQUIRE(ext::make_vorbis_sound(bytes_of(stereo_tone_ogg))->channel_count() == 2);
    }

    SECTION("the duration it was encoded at") {
        REQUIRE(ext::make_vorbis_sound(bytes_of(short_tone_ogg))->duration() == Approx(1.0f).margin(0.05));
        REQUIRE(ext::make_vorbis_sound(bytes_of(long_tone_ogg))->duration() == Approx(6.0f).margin(0.05));
    }

    SECTION("the layout it decodes to") {
        const auto pSound = ext::make_vorbis_sound(bytes_of(short_tone_ogg));

        REQUIRE(pSound->format() == pcm_format::signed_16);
        REQUIRE(pSound->sample_rate() == 8000);
    }
}

TEST_CASE("a vorbis sound's streams decode the tone that was encoded, and decoded now it is the same", "[ext][vorbis]") {
    const auto pSound = ext::make_vorbis_sound(bytes_of(short_tone_ogg));

    const auto all_of = [](const sound &aSound) {
        std::vector<std::int16_t> out;
        std::vector<std::int16_t> chunk(1024);

        auto pStream = aSound.open();

        while (const auto frames = pStream->read(std::as_writable_bytes(std::span(chunk))))
            out.insert(out.end(), chunk.begin(), chunk.begin() + static_cast<std::ptrdiff_t>(frames));

        return out;
    };

    const auto decoded = all_of(*pSound);

    REQUIRE(decoded.size() == Approx(8000).margin(400));

    std::size_t crossings = 0;

    for (std::size_t i = 1; i < decoded.size(); ++i)
        if ((decoded[i - 1] < 0) != (decoded[i] < 0)) ++crossings;

    REQUIRE(crossings == Approx(880).margin(40));

    SECTION("rewound, the same again") {
        REQUIRE(all_of(*pSound) == decoded);
    }

    SECTION("decoded now, every frame the same") {
        const auto pDecoded = make_pcm_sound(*pSound);

        REQUIRE(pDecoded->format() == pcm_format::signed_16);
        REQUIRE(pDecoded->sample_rate() == 8000);
        REQUIRE(all_of(*pDecoded) == decoded);
    }
}

TEST_CASE("a backend plays a vorbis sound as any other: short ones whole, long ones streamed", "[ext][vorbis]") {
    auto pContext = openal_context::make();
    auto pScene = pContext->make_scene();

    auto pShort = pScene->make_emitter(ext::make_vorbis_sound(bytes_of(short_tone_ogg)));
    auto pLong = pScene->make_emitter(ext::make_vorbis_sound(bytes_of(long_tone_ogg)));

    REQUIRE(std::dynamic_pointer_cast<openal_simple_emitter>(pShort) != nullptr);
    REQUIRE(std::dynamic_pointer_cast<openal_stream_emitter>(pLong) != nullptr);

    pShort->play();
    pLong->play();

    REQUIRE(pShort->is_playing());
    REQUIRE(pLong->is_playing());
}
