// © Joseph Cameron - All Rights Reserved

#include <jfc/catch.hpp>

#include <gdk/audio/exception.h>
#include <gdk/audio/ext/capture_buffer.h>

#include <cstddef>
#include <span>
#include <vector>

using namespace gdk::audio;

namespace {
    constexpr std::size_t BYTES_PER_FRAME = 2;

    [[nodiscard]] std::vector<unsigned char> frames(const std::size_t aFrameCount, const unsigned char aFirst = 0) {
        std::vector<unsigned char> out;

        for (std::size_t frame = 0; frame < aFrameCount; ++frame) {
            out.push_back(static_cast<unsigned char>(aFirst + frame));
            out.push_back(static_cast<unsigned char>(aFirst + frame + 1));
        }

        return out;
    }
}

TEST_CASE("a capture buffer holds whole frames up to its capacity", "[ext][capture_buffer]") {
    ext::capture_buffer buffer(4, BYTES_PER_FRAME, false);

    REQUIRE(buffer.capacity_in_frames() == 4);
    REQUIRE(buffer.available_in_frames() == 0);

    const auto three = frames(3);

    REQUIRE_FALSE(buffer.append(three.data(), three.size()));
    REQUIRE(buffer.available_in_frames() == 3);
    REQUIRE_FALSE(buffer.capacity_reached());

    const auto one = frames(1);

    REQUIRE_FALSE(buffer.append(one.data(), one.size()));
    REQUIRE(buffer.capacity_reached());
    REQUIRE(buffer.dropped_frame_count() == 0);

    SECTION("nothing appended is nothing") {
        REQUIRE_FALSE(buffer.append(nullptr, 0));
        REQUIRE(buffer.available_in_frames() == 4);
    }
}

TEST_CASE("a full capture buffer keeps the oldest or the newest, as it was made to", "[ext][capture_buffer]") {
    const auto first = frames(4, 10);
    const auto second = frames(2, 100);

    std::vector<unsigned char> out(4 * BYTES_PER_FRAME);

    SECTION("keeping the oldest, it refuses the newest and says so") {
        ext::capture_buffer buffer(4, BYTES_PER_FRAME, false);

        REQUIRE_FALSE(buffer.append(first.data(), first.size()));
        REQUIRE(buffer.append(second.data(), second.size()));

        REQUIRE(buffer.available_in_frames() == 4);
        REQUIRE(buffer.dropped_frame_count() == 2);

        REQUIRE(buffer.read(std::as_writable_bytes(std::span(out))) == 4);
        REQUIRE(out == first);
    }

    SECTION("dropping the oldest, it makes room") {
        ext::capture_buffer buffer(4, BYTES_PER_FRAME, true);

        REQUIRE_FALSE(buffer.append(first.data(), first.size()));
        REQUIRE_FALSE(buffer.append(second.data(), second.size()));

        REQUIRE(buffer.available_in_frames() == 4);
        REQUIRE(buffer.dropped_frame_count() == 2);

        REQUIRE(buffer.read(std::as_writable_bytes(std::span(out))) == 4);

        std::vector<unsigned char> newest(first.begin() + 2 * BYTES_PER_FRAME, first.end());
        newest.insert(newest.end(), second.begin(), second.end());

        REQUIRE(out == newest);
    }
}

TEST_CASE("a capture buffer hands frames out whole", "[ext][capture_buffer]") {
    ext::capture_buffer buffer(8, BYTES_PER_FRAME, false);

    const auto five = frames(5);

    REQUIRE_FALSE(buffer.append(five.data(), five.size()));

    SECTION("read moves out as many whole frames as fit, oldest first") {
        std::vector<unsigned char> out(3);

        REQUIRE(buffer.read(std::as_writable_bytes(std::span(out))) == 1);
        REQUIRE(out[0] == five[0]);
        REQUIRE(out[1] == five[1]);
        REQUIRE(buffer.available_in_frames() == 4);

        REQUIRE(buffer.read(std::span<std::byte>()) == 0);
        REQUIRE(buffer.available_in_frames() == 4);
    }

    SECTION("take moves out everything") {
        REQUIRE(buffer.take() == five);
        REQUIRE(buffer.available_in_frames() == 0);
        REQUIRE(buffer.take().empty());
    }

    SECTION("clear lets everything go") {
        buffer.clear();

        REQUIRE(buffer.available_in_frames() == 0);
    }
}

TEST_CASE("a microphone request no device could honour is refused, with the same words everywhere", "[ext][capture_buffer]") {
    const microphone::request good{
        .sample_rate = 8000,
        .channel_count = 1,
        .format = pcm_format::signed_16,
        .capacity_in_frames = 100,
    };

    REQUIRE(&ext::validated(good) == &good);

    auto request = good;

    request.channel_count = 0;
    REQUIRE_THROWS_WITH(ext::validated(request), Catch::Contains("no channels"));

    request = good;
    request.sample_rate = 0;
    REQUIRE_THROWS_WITH(ext::validated(request), Catch::Contains("no sample rate"));

    request = good;
    request.capacity_in_frames = 0;
    REQUIRE_THROWS_WITH(ext::validated(request), Catch::Contains("no capacity"));
}
