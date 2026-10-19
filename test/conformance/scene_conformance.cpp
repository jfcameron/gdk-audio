// © Joseph Cameron - All Rights Reserved

#include <jfc/catch.hpp>

#include "conformance.h"
#include "tones.h"

#include <gdk/audio/emitter.h>
#include <gdk/audio/emitter_group.h>
#include <gdk/audio/insert_effect.h>
#include <gdk/audio/listener.h>
#include <gdk/audio/scene.h>
#include <gdk/audio/sound.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <memory>
#include <span>

using namespace gdk::audio;
using gdk::audio::conformance::implementation;
using gdk::audio::conformance::make_context;

namespace {
    struct fixture final {
        context_shared_ptr_type pContext = make_context();
        scene_shared_ptr_type pScene = pContext->make_scene();
    };

    class square_wave_sound final : public sound {
        static constexpr std::size_t RATE = 8000;
        static constexpr std::size_t FRAMES = RATE / 2;

        class stream final : public pcm_stream {
            std::size_t mCursor = 0;

        public:
            std::size_t read(std::span<std::byte> aOut) override {
                const auto frames = std::min(aOut.size() / sizeof(std::int16_t), FRAMES - mCursor);
                auto *const out = reinterpret_cast<std::int16_t *>(aOut.data());

                for (std::size_t i = 0; i < frames; ++i) out[i] = ((mCursor + i) / 32) % 2 ? 8000 : -8000;

                mCursor += frames;

                return frames;
            }

            void rewind() override { mCursor = 0; }
        };

    public:
        [[nodiscard]] std::size_t channel_count() const override { return 1; }

        [[nodiscard]] std::size_t sample_rate() const override { return RATE; }

        [[nodiscard]] pcm_format format() const override { return pcm_format::signed_16; }

        [[nodiscard]] audio_floating_point_type duration() const override {
            return static_cast<audio_floating_point_type>(FRAMES) / RATE;
        }

        [[nodiscard]] std::unique_ptr<pcm_stream> open() const override { return std::make_unique<stream>(); }
    };

    class counting_sound final : public sound {
    public:
        static constexpr std::size_t RATE = 8000;
        static constexpr std::size_t FRAMES = 1000;

        mutable std::size_t opened = 0;

        [[nodiscard]] std::size_t channel_count() const override { return 2; }

        [[nodiscard]] std::size_t sample_rate() const override { return RATE; }

        [[nodiscard]] pcm_format format() const override { return pcm_format::signed_16; }

        [[nodiscard]] audio_floating_point_type duration() const override {
            return static_cast<audio_floating_point_type>(FRAMES) / RATE;
        }

        [[nodiscard]] std::unique_ptr<pcm_stream> open() const override {
            ++opened;

            return std::make_unique<ramp>();
        }

    private:
        class ramp final : public pcm_stream {
            std::size_t mCursor = 0;

        public:
            std::size_t read(std::span<std::byte> aOut) override {
                const auto frames = std::min({aOut.size() / (2 * sizeof(std::int16_t)), FRAMES - mCursor, std::size_t{3}});
                auto *const out = reinterpret_cast<std::int16_t *>(aOut.data());

                for (std::size_t i = 0; i < frames; ++i) {
                    out[i * 2] = static_cast<std::int16_t>(mCursor + i);
                    out[i * 2 + 1] = static_cast<std::int16_t>(-static_cast<int>(mCursor + i));
                }

                mCursor += frames;

                return frames;
            }

            void rewind() override { mCursor = 0; }
        };
    };
}

TEST_CASE("conformance, make_emitter refuses what it cannot play", "[conformance][scene][emitter]") {
    INFO("implementation " << implementation());

    fixture f;

    SECTION("a sound implemented outside the library is accepted") {
        REQUIRE(f.pScene->make_emitter(std::make_shared<square_wave_sound>()) != nullptr);
    }

    SECTION("a null sound is refused, and not added") {
        REQUIRE_THROWS_WITH(f.pScene->make_emitter(nullptr), Catch::Contains("null sound"));
        REQUIRE_THROWS_WITH(f.pScene->make_emitter(nullptr, insert_effect_collection_type{}),
            Catch::Contains("null sound"));

        REQUIRE(f.pScene->emitter_count() == 0);
    }

    SECTION("a null insert in the chain is refused") {
        REQUIRE_THROWS_WITH(f.pScene->make_emitter(short_tone(), {nullptr}),
            Catch::Contains("insert effect in the chain was null"));
        REQUIRE_THROWS_WITH(f.pScene->make_emitter(short_tone(), {make_lowpass_insert(800.0f, 8000), nullptr}),
            Catch::Contains("insert effect in the chain was null"));

        REQUIRE(f.pScene->emitter_count() == 0);
    }

    SECTION("a sound placed a channel at a time needs a place for every channel, and no more") {
        const std::array<audio_vector3_type, 2> places{{{-1, 0, 0}, {1, 0, 0}}};

        REQUIRE_THROWS_WITH(f.pScene->make_emitter(stereo_tone(), std::span(places).first(1)),
            Catch::Contains("a place for each"));
        REQUIRE_THROWS_WITH(f.pScene->make_emitter(short_tone(), std::span(places)), Catch::Contains("a place for each"));
        REQUIRE_THROWS_WITH(f.pScene->make_emitter(short_tone(), std::span<const audio_vector3_type>()),
            Catch::Contains("a place for each"));

        REQUIRE(f.pScene->emitter_count() == 0);

        REQUIRE(f.pScene->make_emitter(stereo_tone(), std::span(places)) != nullptr);
    }
}

TEST_CASE("conformance, a scene holds its emitters weakly", "[conformance][scene][emitter]") {
    INFO("implementation " << implementation());

    fixture f;

    SECTION("making one adds it, and adds no reference to it") {
        const auto pEmitter = f.pScene->make_emitter(short_tone());

        REQUIRE(f.pScene->emitter_count() == 1);
        REQUIRE(pEmitter.use_count() == 1);
    }

    SECTION("dropping the caller's handle destroys the emitter, playing or not") {
        std::weak_ptr<emitter> weak;

        {
            const auto pEmitter = f.pScene->make_emitter(short_tone());
            weak = pEmitter;
            pEmitter->play();

            REQUIRE_FALSE(weak.expired());
        }

        REQUIRE(weak.expired());
        REQUIRE(f.pScene->emitter_count() == 0);
    }

    SECTION("update sweeps the expired away") {
        {
            const auto pEmitter = f.pScene->make_emitter(short_tone());
        }

        REQUIRE_NOTHROW(f.pScene->update());
        REQUIRE(f.pScene->emitter_count() == 0);
    }

    SECTION("remove silences an emitter the caller still holds") {
        const auto pEmitter = f.pScene->make_emitter(short_tone());
        pEmitter->play();

        f.pScene->remove(pEmitter);

        REQUIRE_FALSE(pEmitter->is_playing());
        REQUIRE(f.pScene->emitter_count() == 0);
    }

    SECTION("removing something the scene does not hold is harmless") {
        REQUIRE_NOTHROW(f.pScene->remove(nullptr));
        REQUIRE(f.pScene->emitter_count() == 0);
    }
}

TEST_CASE("conformance, a scene's listener", "[conformance][scene][listener]") {
    INFO("implementation " << implementation());

    fixture f;

    auto &ears = f.pScene->get_listener();

    SECTION("starts at the origin, unrotated, still and whole") {
        REQUIRE(ears.position() == audio_vector3_type::zero);
        REQUIRE(ears.rotation() == audio_quaternion_type::identity);
        REQUIRE(ears.velocity() == audio_vector3_type::zero);
        REQUIRE(ears.gain() == Approx(1.0f));
    }

    SECTION("position, velocity and gain round trip") {
        ears.set_position({1, -2, 3});
        ears.set_velocity({0, 0, -4});
        ears.set_gain(0.25f);

        REQUIRE(ears.position() == audio_vector3_type(1, -2, 3));
        REQUIRE(ears.velocity() == audio_vector3_type(0, 0, -4));
        REQUIRE(ears.gain() == Approx(0.25f));
    }

    SECTION("set_transform sets the position and the rotation") {
        const auto quarterTurn = audio_quaternion_type::from_euler({0, 1.5708f, 0});
        ears.set_transform({5, 6, 7}, quarterTurn);

        REQUIRE(ears.position() == audio_vector3_type(5, 6, 7));
        REQUIRE(ears.rotation() == quarterTurn);
    }

    SECTION("it is the same object every time it is asked for") {
        ears.set_position({9, 9, 9});

        REQUIRE(f.pScene->get_listener().position() == audio_vector3_type(9, 9, 9));
    }
}

TEST_CASE("conformance, scenes share neither listeners nor emitters", "[conformance][scene]") {
    INFO("implementation " << implementation());

    const auto pContext = make_context();

    const auto pFirst = pContext->make_scene();
    const auto pSecond = pContext->make_scene();

    SECTION("moving one listener leaves the other") {
        pFirst->get_listener().set_position({100, 0, 0});

        REQUIRE(pSecond->get_listener().position() == audio_vector3_type::zero);
    }

    SECTION("an emitter is the scene's that made it") {
        const auto pSound = short_tone();
        const auto pEmitter = pFirst->make_emitter(pSound);

        REQUIRE(pFirst->emitter_count() == 1);
        REQUIRE(pSecond->emitter_count() == 0);

        const auto pOther = pSecond->make_emitter(pSound);

        REQUIRE(pOther != pEmitter);
        REQUIRE(pSecond->emitter_count() == 1);

        pSecond->remove(pEmitter);

        REQUIRE(pFirst->emitter_count() == 1);
    }
}

TEST_CASE("conformance, update is repeatable and disturbs nothing", "[conformance][scene]") {
    INFO("implementation " << implementation());

    fixture f;

    for (int i = 0; i < 4; ++i) REQUIRE_NOTHROW(f.pScene->update());

    const auto pEmitter = f.pScene->make_emitter(long_tone());
    pEmitter->set_position({3, 0, 0});
    pEmitter->play();

    for (int i = 0; i < 8; ++i) REQUIRE_NOTHROW(f.pScene->update());

    REQUIRE(f.pScene->emitter_count() == 1);
    REQUIRE(pEmitter->position() == audio_vector3_type(3, 0, 0));
    REQUIRE(pEmitter->is_playing());
}

TEST_CASE("conformance, emitters in a group start, stop, pause and resume as one", "[conformance][group]") {
    INFO("implementation " << implementation());

    fixture f;

    const auto pSound = long_tone();

    const std::array<emitter_shared_ptr_type, 3> members{
        f.pScene->make_emitter(pSound), f.pScene->make_emitter(pSound), f.pScene->make_emitter(pSound)};

    const auto pGroup = f.pScene->make_emitter_group(std::span(members));

    REQUIRE(pGroup->size() == 3);
    REQUIRE_FALSE(pGroup->is_playing());

    SECTION("together") {
        pGroup->play();

        REQUIRE(pGroup->is_playing());
        for (const auto &pMember : members) REQUIRE(pMember->is_playing());

        pGroup->pause();

        REQUIRE(pGroup->is_paused());
        REQUIRE_FALSE(pGroup->is_playing());
        for (const auto &pMember : members) REQUIRE(pMember->is_paused());

        pGroup->resume();

        REQUIRE(pGroup->is_playing());
        for (const auto &pMember : members) REQUIRE(pMember->is_playing());

        pGroup->stop();

        REQUIRE_FALSE(pGroup->is_playing());
        for (const auto &pMember : members) REQUIRE_FALSE(pMember->is_playing());
    }

    SECTION("a member that is let go of leaves the group") {
        auto pExtra = f.pScene->make_emitter(pSound);

        const auto pSmall = f.pScene->make_emitter_group(std::array{pExtra, members[0]});

        REQUIRE(pSmall->size() == 2);

        pExtra.reset();

        REQUIRE(pSmall->size() == 1);
    }

    SECTION("an emitter of another scene is refused") {
        const auto pOther = f.pContext->make_scene();
        const std::array<emitter_shared_ptr_type, 1> foreign{pOther->make_emitter(pSound)};

        REQUIRE_THROWS_WITH(f.pScene->make_emitter_group(std::span(foreign)), Catch::Contains("this scene's emitters"));
    }
}

TEST_CASE("conformance, a sound decoded now is never opened again by its emitters", "[conformance][sound]") {
    INFO("implementation " << implementation());

    fixture f;

    const counting_sound original;

    const auto pDecoded = make_pcm_sound(original);

    REQUIRE(original.opened == 1);

    for (int each = 0; each < 3; ++each) f.pScene->make_emitter(pDecoded)->play();

    REQUIRE(original.opened == 1);
}
