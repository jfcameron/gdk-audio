// © Joseph Cameron - All Rights Reserved

#include <jfc/catch.hpp>

#include <gdk/audio/exception.h>
#include <gdk/audio/emitter.h>
#include <gdk/audio/openal_context.h>
#include <gdk/audio/openal_emitter.h>
#include <gdk/audio/openal_scene.h>
#include <gdk/audio/scene.h>
#include <gdk/audio/send_effect.h>
#include <gdk/audio/sound.h>

#include "data/short_tone_ogg.h"

#define AL_ALEXT_PROTOTYPES
#include <AL/al.h>
#include <AL/efx.h>

#include <memory>

using namespace gdk;
using namespace gdk::audio;

namespace {
    struct fixture final {
        context_unique_ptr_type pContext = openal_context::make();
        scene_shared_ptr_type pScene = pContext->make_scene();

        [[nodiscard]] emitter_shared_ptr_type emitter() const {
            return pScene->make_emitter(make_vorbis_sound(short_tone_ogg, sizeof(short_tone_ogg)));
        }

        [[nodiscard]] ALuint slot() const {
            return std::dynamic_pointer_cast<openal_scene>(pScene)->effect_slot_handle();
        }
    };

    [[nodiscard]] ALint effect_type_in_slot(const ALuint aSlot) {
        ALint effect = AL_EFFECT_NULL;
        alGetAuxiliaryEffectSloti(aSlot, AL_EFFECTSLOT_EFFECT, &effect);

        if (effect == AL_EFFECT_NULL) return AL_EFFECT_NULL;

        ALint type = AL_EFFECT_NULL;
        alGetEffecti(static_cast<ALuint>(effect), AL_EFFECT_TYPE, &type);

        return type;
    }

    template <typename work_type>
    [[nodiscard]] bool without_al_error(work_type aWork) {
        while (alGetError() != AL_NO_ERROR) {}

        aWork();

        return alGetError() == AL_NO_ERROR;
    }

    [[nodiscard]] ALuint filter_of(const emitter_shared_ptr_type &aEmitter) {
        return std::dynamic_pointer_cast<openal_emitter>(aEmitter)->lowpass_filter_handle();
    }

    [[nodiscard]] ALuint send_of(const emitter_shared_ptr_type &aEmitter) {
        return std::dynamic_pointer_cast<openal_emitter>(aEmitter)->send_slot();
    }

    [[nodiscard]] ALint filter_property(const ALuint aFilter, const ALenum aParameter) {
        ALint value = -1;
        alGetFilteri(aFilter, aParameter, &value);

        return value;
    }

    [[nodiscard]] float filter_gain(const ALuint aFilter, const ALenum aParameter) {
        ALfloat value = -1;
        alGetFilterf(aFilter, aParameter, &value);

        return value;
    }
}

TEST_CASE("a scene starts dry", "[send]") {
    fixture f;

    REQUIRE_FALSE(f.pScene->has_send_effect());
    REQUIRE(f.slot() == AL_EFFECTSLOT_NULL);
}

TEST_CASE("a scene can be given a room", "[send]") {
    fixture f;

    SECTION("reverb reaches the slot as reverb") {
        f.pScene->set_reverb({});

        REQUIRE(f.pScene->has_send_effect());
        REQUIRE(f.slot() != AL_EFFECTSLOT_NULL);
        REQUIRE(effect_type_in_slot(f.slot()) == AL_EFFECT_REVERB);
    }

    SECTION("echo reaches it as echo") {
        f.pScene->set_echo({});

        REQUIRE(effect_type_in_slot(f.slot()) == AL_EFFECT_ECHO);
    }

    SECTION("one replaces the other in the same slot") {
        f.pScene->set_reverb({});
        const auto first = f.slot();

        f.pScene->set_echo({});

        REQUIRE(f.slot() == first);
        REQUIRE(effect_type_in_slot(f.slot()) == AL_EFFECT_ECHO);
    }

    SECTION("non default parameters are accepted without complaint") {
        reverb_parameters cathedral;
        cathedral.decay_seconds = 8.0f;
        cathedral.gain = 0.5f;
        cathedral.diffusion = 0.9f;

        REQUIRE(without_al_error([&] { f.pScene->set_reverb(cathedral); }));
        REQUIRE(effect_type_in_slot(f.slot()) == AL_EFFECT_REVERB);
    }
}

TEST_CASE("every emitter in the scene reaches the send", "[send]") {
    fixture f;

    SECTION("emitters that predate the effect are wired retroactively") {
        auto pEmitter = f.emitter();

        REQUIRE(send_of(pEmitter) == AL_EFFECTSLOT_NULL);

        f.pScene->set_reverb({});

        REQUIRE(send_of(pEmitter) == f.slot());
        REQUIRE(send_of(pEmitter) != AL_EFFECTSLOT_NULL);
    }

    SECTION("emitters made afterwards join it") {
        f.pScene->set_reverb({});

        REQUIRE(send_of(f.emitter()) == f.slot());
    }

    SECTION("they share the one slot rather than getting one each") {
        f.pScene->set_reverb({});

        auto pFirst = f.emitter();
        auto pSecond = f.emitter();

        REQUIRE(send_of(pFirst) == send_of(pSecond));
        REQUIRE(send_of(pFirst) != AL_EFFECTSLOT_NULL);
    }

    SECTION("the routing is accepted by the device, not merely recorded") {
        auto pEmitter = f.emitter();

        REQUIRE(without_al_error([&] { f.pScene->set_reverb({}); }));
    }
}

TEST_CASE("clearing a send releases the slot", "[send]") {
    fixture f;

    auto pEmitter = f.emitter();

    f.pScene->set_reverb({});
    REQUIRE(f.slot() != AL_EFFECTSLOT_NULL);

    REQUIRE(without_al_error([&] { f.pScene->clear_send_effect(); }));

    REQUIRE(send_of(pEmitter) == AL_EFFECTSLOT_NULL);

    REQUIRE_FALSE(f.pScene->has_send_effect());
    REQUIRE(f.slot() == AL_EFFECTSLOT_NULL);
}

TEST_CASE("clearing a scene that has no send is harmless", "[send]") {
    fixture f;

    REQUIRE(without_al_error([&] { f.pScene->clear_send_effect(); }));
    REQUIRE_FALSE(f.pScene->has_send_effect());
}

TEST_CASE("a send belongs to one scene", "[send]") {
    auto pContext = openal_context::make();

    auto pWet = pContext->make_scene();
    auto pDry = pContext->make_scene();

    pWet->set_reverb({});

    REQUIRE(pWet->has_send_effect());
    REQUIRE_FALSE(pDry->has_send_effect());

    REQUIRE(std::dynamic_pointer_cast<openal_scene>(pWet)->effect_slot_handle()
        != std::dynamic_pointer_cast<openal_scene>(pDry)->effect_slot_handle());
}

TEST_CASE("lowpass is per emitter", "[send][emitter]") {
    fixture f;

    auto pMuffled = f.emitter();
    auto pClear = f.emitter();

    SECTION("it starts absent") {
        REQUIRE(filter_of(pMuffled) == AL_FILTER_NULL);
    }

    SECTION("setting it builds a lowpass and affects only that emitter") {
        REQUIRE(without_al_error([&] { pMuffled->set_lowpass({1.0f, 0.05f}); }));

        const auto filter = filter_of(pMuffled);

        REQUIRE(filter != AL_FILTER_NULL);
        REQUIRE(filter_property(filter, AL_FILTER_TYPE) == AL_FILTER_LOWPASS);
        REQUIRE(filter_of(pClear) == AL_FILTER_NULL);
    }

    SECTION("the parameters reach the filter") {
        pMuffled->set_lowpass({0.8f, 0.05f});

        const auto filter = filter_of(pMuffled);

        REQUIRE(filter_gain(filter, AL_LOWPASS_GAIN) == Approx(0.8f));
        REQUIRE(filter_gain(filter, AL_LOWPASS_GAINHF) == Approx(0.05f));
    }

    SECTION("it can be cleared") {
        pMuffled->set_lowpass({1.0f, 0.05f});
        REQUIRE(filter_of(pMuffled) != AL_FILTER_NULL);

        REQUIRE(without_al_error([&] { pMuffled->clear_lowpass(); }));

        REQUIRE(filter_of(pMuffled) == AL_FILTER_NULL);
    }

    SECTION("setting it twice reuses the one filter") {
        pMuffled->set_lowpass({1.0f, 0.5f});
        const auto first = filter_of(pMuffled);

        REQUIRE(first != AL_FILTER_NULL);

        pMuffled->set_lowpass({1.0f, 0.1f});

        REQUIRE(filter_of(pMuffled) == first);
        REQUIRE(filter_gain(first, AL_LOWPASS_GAINHF) == Approx(0.1f));
    }

    SECTION("it is independent of the scene's send") {
        f.pScene->set_reverb({});
        pMuffled->set_lowpass({1.0f, 0.05f});

        REQUIRE(f.pScene->has_send_effect());
        REQUIRE(filter_of(pMuffled) != AL_FILTER_NULL);
    }
}
