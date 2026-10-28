// © Joseph Cameron - MIT License

#include <jfc/catch.hpp>

#include <gdk/audio/exception.h>
#include <gdk/audio/insert_effect.h>

#include <algorithm>
#include <span>
#include <vector>

using namespace gdk::audio;

TEST_CASE("the shipped inserts", "[insert]") {
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
