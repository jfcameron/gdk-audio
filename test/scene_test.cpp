// © Joseph Cameron - All Rights Reserved

#include <jfc/catch.hpp>

#include <gdk/audio/exception.h>
#include <gdk/audio/emitter.h>
#include <gdk/audio/listener.h>
#include <gdk/audio/openal_context.h>
#include <gdk/audio/openal_emitter.h>
#include <gdk/audio/scene.h>
#include <gdk/audio/sound.h>

#include "data/long_tone_ogg.h"
#include "data/short_tone_ogg.h"

#include <gdk/math_constants.h>

#include <AL/al.h>

#include <algorithm>
#include <cmath>
#include <memory>
#include <span>

using namespace gdk;
using namespace gdk::audio;

namespace {
    [[nodiscard]] sound_shared_ptr_type sound_from(const unsigned char *const aData,
        const std::size_t aSize) {
        return make_vorbis_sound(aData, aSize);
    }

    struct fixture final {
        context_unique_ptr_type pContext = openal_context::make();
        scene_shared_ptr_type pScene = pContext->make_scene();

        [[nodiscard]] emitter_shared_ptr_type emitter_for(const unsigned char *const aData,
            const std::size_t aSize) {
            return pScene->make_emitter(sound_from(aData, aSize));
        }
    };

    class square_wave_sound final : public sound {
        static constexpr std::size_t RATE = 8000;
        static constexpr std::size_t FRAMES = RATE / 2;   

        class stream final : public pcm_stream {
            std::size_t m_Cursor = 0;

        public:
            std::size_t read(std::span<std::byte> aOut) override {
                const auto frames = std::min(aOut.size() / sizeof(short), FRAMES - m_Cursor);
                auto *const out = reinterpret_cast<short *>(aOut.data());

                for (std::size_t i = 0; i < frames; ++i)
                    out[i] = ((m_Cursor + i) / 32) % 2 ? 8000 : -8000;

                m_Cursor += frames;

                return frames;
            }

            void rewind() override { m_Cursor = 0; }
        };

    public:
        [[nodiscard]] std::size_t channel_count() const override { return 1; }

        [[nodiscard]] std::size_t sample_rate() const override { return RATE; }

        [[nodiscard]] pcm_format format() const override { return pcm_format::signed_16; }

        [[nodiscard]] audio_floating_point_type duration() const override {
            return static_cast<audio_floating_point_type>(FRAMES) / RATE;
        }

        [[nodiscard]] std::unique_ptr<pcm_stream> open() const override {
            return std::unique_ptr<pcm_stream>(new stream());
        }
    };
}

TEST_CASE("make_emitter rejects what it cannot decode", "[scene][emitter]") {
    fixture f;

    SECTION("a sound implemented outside the library is accepted") {
        REQUIRE(f.pScene->make_emitter(std::make_shared<square_wave_sound>()) != nullptr);
    }

    SECTION("a null sound throws") {
        REQUIRE_THROWS_AS(f.pScene->make_emitter(nullptr), exception);
    }

    SECTION("a rejected emitter is not added to the scene") {
        REQUIRE_THROWS(f.pScene->make_emitter(nullptr));

        REQUIRE(f.pScene->emitter_count() == 0);
    }
}

TEST_CASE("the scene refers to its emitters weakly", "[scene][emitter]") {
    fixture f;

    SECTION("making one adds it, and adds no reference to it") {
        auto pEmitter = f.emitter_for(short_tone_ogg, sizeof(short_tone_ogg));

        REQUIRE(f.pScene->emitter_count() == 1);
        REQUIRE(pEmitter.use_count() == 1);
    }

    SECTION("dropping the caller's handle destroys the emitter") {
        std::weak_ptr<emitter> weak;

        {
            auto pEmitter = f.emitter_for(short_tone_ogg, sizeof(short_tone_ogg));
            weak = pEmitter;
            pEmitter->play();

            REQUIRE_FALSE(weak.expired());
        }

        REQUIRE(weak.expired());
    }

    SECTION("the scene stops reporting an emitter nobody holds") {
        {
            auto pEmitter = f.emitter_for(short_tone_ogg, sizeof(short_tone_ogg));

            REQUIRE(f.pScene->emitter_count() == 1);
        }

        REQUIRE(f.pScene->emitter_count() == 0);
    }

    SECTION("update sweeps the expired entries away") {
        {
            auto pEmitter = f.emitter_for(short_tone_ogg, sizeof(short_tone_ogg));
        }

        REQUIRE_NOTHROW(f.pScene->update());
        REQUIRE(f.pScene->emitter_count() == 0);
    }

    SECTION("remove silences an emitter the caller still holds") {
        auto pEmitter = f.emitter_for(short_tone_ogg, sizeof(short_tone_ogg));
        pEmitter->play();

        f.pScene->remove(pEmitter);

        REQUIRE(pEmitter != nullptr);
        REQUIRE_FALSE(pEmitter->is_playing());
        REQUIRE(f.pScene->emitter_count() == 0);
    }

    SECTION("removing something the scene does not hold is harmless") {
        REQUIRE_NOTHROW(f.pScene->remove(nullptr));
        REQUIRE(f.pScene->emitter_count() == 0);
    }
}

TEST_CASE("a scene's listener", "[scene][listener]") {
    fixture f;

    auto &ears = f.pScene->get_listener();

    SECTION("starts at the origin, unrotated and still") {
        REQUIRE(ears.position() == audio_vector3_type::zero);
        REQUIRE(ears.rotation() == audio_quaternion_type::identity);
        REQUIRE(ears.velocity() == audio_vector3_type::zero);
    }

    SECTION("position round trips") {
        ears.set_position({1, -2, 3});

        REQUIRE(ears.position() == audio_vector3_type(1, -2, 3));
    }

    SECTION("velocity round trips") {
        ears.set_velocity({0, 0, -4});

        REQUIRE(ears.velocity() == audio_vector3_type(0, 0, -4));
    }

    SECTION("rotation round trips") {
        const auto quarterTurn = audio_quaternion_type::from_euler({0, 1.5708f, 0});
        ears.set_rotation(quarterTurn);

        REQUIRE(ears.rotation() == quarterTurn);
    }

    SECTION("set_transform sets both") {
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

TEST_CASE("scenes do not share a listener", "[scene][listener]") {
    auto pContext = openal_context::make();

    auto pFirst = pContext->make_scene();
    auto pSecond = pContext->make_scene();

    pFirst->get_listener().set_position({100, 0, 0});

    REQUIRE(pFirst->get_listener().position() == audio_vector3_type(100, 0, 0));
    REQUIRE(pSecond->get_listener().position() == audio_vector3_type::zero);
}

TEST_CASE("emitters belong to the scene that made them", "[scene][emitter]") {
    auto pContext = openal_context::make();

    auto pFirst = pContext->make_scene();
    auto pSecond = pContext->make_scene();

    auto pSound = sound_from(short_tone_ogg, sizeof(short_tone_ogg));

    auto pEmitter = pFirst->make_emitter(pSound);

    REQUIRE(pFirst->emitter_count() == 1);
    REQUIRE(pSecond->emitter_count() == 0);

    SECTION("one sound can feed emitters in several scenes") {
        auto pOther = pSecond->make_emitter(pSound);

        REQUIRE(pOther != pEmitter);
        REQUIRE(pFirst->emitter_count() == 1);
        REQUIRE(pSecond->emitter_count() == 1);
    }

    SECTION("removing from the wrong scene does nothing") {
        pSecond->remove(pEmitter);

        REQUIRE(pFirst->emitter_count() == 1);
    }
}

TEST_CASE("update is repeatable and does not disturb the scene", "[scene]") {
    fixture f;

    auto pEmitter = f.emitter_for(long_tone_ogg, sizeof(long_tone_ogg));
    pEmitter->set_position({3, 0, 0});
    pEmitter->play();

    for (int i = 0; i < 8; ++i) REQUIRE_NOTHROW(f.pScene->update());

    REQUIRE(f.pScene->emitter_count() == 1);
    REQUIRE(pEmitter->position() == audio_vector3_type(3, 0, 0));
}

TEST_CASE("an empty scene updates cleanly", "[scene]") {
    fixture f;

    for (int i = 0; i < 4; ++i) REQUIRE_NOTHROW(f.pScene->update());

    REQUIRE(f.pScene->emitter_count() == 0);
}

namespace {
    [[nodiscard]] audio_vector3_type al_position_of(const emitter_shared_ptr_type &aEmitter) {
        const auto pImplementation = std::dynamic_pointer_cast<openal_emitter>(aEmitter);

        REQUIRE(pImplementation != nullptr);

        ALfloat x = 0, y = 0, z = 0;
        alGetSource3f(pImplementation->source_handle(), AL_POSITION, &x, &y, &z);

        return {x, y, z};
    }

    [[nodiscard]] bool near(const audio_vector3_type &a, const audio_vector3_type &b) {
        return std::abs(a.x - b.x) < 1e-4f && std::abs(a.y - b.y) < 1e-4f
            && std::abs(a.z - b.z) < 1e-4f;
    }
}

TEST_CASE("emitters reach the device in listener space", "[scene][listener]") {
    fixture f;

    auto pEmitter = f.emitter_for(short_tone_ogg, sizeof(short_tone_ogg));
    auto &ears = f.pScene->get_listener();

    SECTION("with the listener at the origin, world space passes through") {
        pEmitter->set_position({1, 2, 3});
        f.pScene->update();

        REQUIRE(near(al_position_of(pEmitter), {1, 2, 3}));
    }

    SECTION("moving the listener onto the emitter puts it at the ear") {
        pEmitter->set_position({10, 0, 0});
        ears.set_position({10, 0, 0});
        f.pScene->update();

        REQUIRE(near(al_position_of(pEmitter), audio_vector3_type::zero));
    }

    SECTION("the listener's position is subtracted, not added") {
        pEmitter->set_position({4, 0, 0});
        ears.set_position({1, 0, 0});
        f.pScene->update();

        REQUIRE(near(al_position_of(pEmitter), {3, 0, 0}));
    }

    SECTION("the listener's rotation is applied as its inverse") {
        pEmitter->set_position({0, 0, -10});
        ears.set_rotation(audio_quaternion_type::from_euler({0, numbers::pi_f / 2, 0}));
        f.pScene->update();

        const auto heard = al_position_of(pEmitter);

        REQUIRE(std::abs(heard.y) < 1e-4f);
        REQUIRE(std::abs(heard.z) < 1e-3f);
        REQUIRE(heard.x > 9.0f);
    }

    SECTION("velocity is relative, so shared motion produces no shift") {
        pEmitter->set_velocity({5, 0, 0});
        ears.set_velocity({5, 0, 0});
        f.pScene->update();

        ALfloat x = 0, y = 0, z = 0;
        alGetSource3f(std::dynamic_pointer_cast<openal_emitter>(pEmitter)->source_handle(),
            AL_VELOCITY, &x, &y, &z);

        REQUIRE(near({x, y, z}, audio_vector3_type::zero));
    }

    SECTION("every source is flagged listener relative") {
        ALint relative = AL_FALSE;
        alGetSourcei(std::dynamic_pointer_cast<openal_emitter>(pEmitter)->source_handle(),
            AL_SOURCE_RELATIVE, &relative);

        REQUIRE(relative == AL_TRUE);
    }
}

TEST_CASE("emitter properties reach the device", "[emitter][listener]") {
    fixture f;

    auto pEmitter = f.emitter_for(short_tone_ogg, sizeof(short_tone_ogg));
    const auto source = std::dynamic_pointer_cast<openal_emitter>(pEmitter)->source_handle();

    const auto al_float = [&source](const ALenum aParameter) {
        ALfloat value = 0;
        alGetSourcef(source, aParameter, &value);

        return value;
    };

    SECTION("pitch round trips through OpenAL rather than a local copy") {
        pEmitter->set_pitch(1.5f);

        REQUIRE(pEmitter->pitch() == Approx(1.5f));
        REQUIRE(al_float(AL_PITCH) == Approx(1.5f));
    }

    SECTION("attenuation is handed straight over") {
        pEmitter->set_attenuation(2.0f, 40.0f, 0.75f);

        REQUIRE(al_float(AL_REFERENCE_DISTANCE) == Approx(2.0f));
        REQUIRE(al_float(AL_MAX_DISTANCE) == Approx(40.0f));
        REQUIRE(al_float(AL_ROLLOFF_FACTOR) == Approx(0.75f));
    }

    SECTION("cone angles are converted to degrees") {
        pEmitter->set_cone(numbers::pi_f / 2, numbers::pi_f, 0.25f);

        REQUIRE(al_float(AL_CONE_INNER_ANGLE) == Approx(90.0f));
        REQUIRE(al_float(AL_CONE_OUTER_ANGLE) == Approx(180.0f));
        REQUIRE(al_float(AL_CONE_OUTER_GAIN) == Approx(0.25f));
    }

    SECTION("gain defaults to unattenuated") {
        f.pScene->update();

        REQUIRE(pEmitter->gain() == Approx(1.0f));
        REQUIRE(al_float(AL_GAIN) == Approx(1.0f));
    }

    SECTION("the listener's gain is folded into the emitter's") {
        pEmitter->set_gain(0.5f);
        f.pScene->get_listener().set_gain(0.5f);
        f.pScene->update();

        REQUIRE(pEmitter->gain() == Approx(0.5f));
        REQUIRE(f.pScene->get_listener().gain() == Approx(0.5f));
        REQUIRE(al_float(AL_GAIN) == Approx(0.25f));
    }

    SECTION("a silent listener silences its scene without touching the emitters") {
        pEmitter->set_gain(1.0f);
        f.pScene->get_listener().set_gain(0);
        f.pScene->update();

        REQUIRE(pEmitter->gain() == Approx(1.0f));
        REQUIRE(al_float(AL_GAIN) == Approx(0.0f));
    }
}

TEST_CASE("direction rotates with the listener but does not translate", "[emitter][listener]") {
    fixture f;

    auto pEmitter = f.emitter_for(short_tone_ogg, sizeof(short_tone_ogg));
    const auto source = std::dynamic_pointer_cast<openal_emitter>(pEmitter)->source_handle();

    const auto al_direction = [&source]() {
        ALfloat x = 0, y = 0, z = 0;
        alGetSource3f(source, AL_DIRECTION, &x, &y, &z);

        return audio_vector3_type(x, y, z);
    };

    SECTION("it is unchanged when the listener is at rest") {
        pEmitter->set_direction({0, 0, -1});
        f.pScene->update();

        REQUIRE(near(al_direction(), {0, 0, -1}));
    }

    SECTION("moving the listener leaves it alone") {
        pEmitter->set_direction({0, 0, -1});
        f.pScene->get_listener().set_position({100, 50, -25});
        f.pScene->update();

        REQUIRE(near(al_direction(), {0, 0, -1}));
    }

    SECTION("turning the listener turns it") {
        pEmitter->set_direction({0, 0, -1});
        f.pScene->get_listener().set_rotation(
            audio_quaternion_type::from_euler({0, numbers::pi_f / 2, 0}));
        f.pScene->update();

        const auto heard = al_direction();

        REQUIRE(heard.x > 0.99f);
        REQUIRE(std::abs(heard.z) < 1e-3f);
    }

    SECTION("omnidirectional is the default and survives a transform") {
        f.pScene->get_listener().set_transform({7, 8, 9},
            audio_quaternion_type::from_euler({0.3f, 0.4f, 0.5f}));
        f.pScene->update();

        REQUIRE(near(al_direction(), audio_vector3_type::zero));
    }
}
