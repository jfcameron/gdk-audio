// © Joseph Cameron - All Rights Reserved

#include <gdk/audio/exception.h>
#include <gdk/audio/impl_efx.h>
#include <gdk/audio/openal_scene.h>
#include <gdk/audio/openal_simple_emitter.h>
#include <gdk/audio/openal_stream_emitter.h>

#include <algorithm>
#include <functional>
#include <memory>

namespace gdk::audio
{
    namespace
    {
        template <typename weak_ptr_container_type>
        auto lock_and_prune(weak_ptr_container_type &aContainer) {
            using weak_ptr_type = typename weak_ptr_container_type::value_type;
            using element_type = typename weak_ptr_type::element_type;

            std::vector<std::shared_ptr<element_type>> result;
            result.reserve(aContainer.size());

            auto it = aContainer.begin();
            while (it != aContainer.end()) {
                if (auto sp = it->lock()) {
                    result.push_back(std::move(sp));
                    ++it;
                }
                else it = aContainer.erase(it);
            }

            return result;
        }

        [[nodiscard]] const efx_functions &efx_or_throw() {
            const auto *const pEfx = efx();

            if (!pEfx) throw exception("this openal has no effects extension (ALC_EXT_EFX), so a scene "
                "cannot have a reverb or an echo; an insert effect, make_echo_insert, echoes without it");

            return *pEfx;
        }
    }

    scene_shared_ptr_type openal_scene::make(openal_policy aPolicy)
    {
        return scene_shared_ptr_type(new openal_scene(std::move(aPolicy)));
    }

    openal_scene::openal_scene(openal_policy aPolicy)
    : m_Policy(std::move(aPolicy))
    {}

    listener &openal_scene::get_listener()
    {
        return m_Listener;
    }

    const listener &openal_scene::get_listener() const
    {
        return m_Listener;
    }

    emitter_shared_ptr_type openal_scene::make_emitter(sound_shared_ptr_type apSound)
    {
        return add_emitter(std::move(apSound), {}, {});
    }

    emitter_shared_ptr_type openal_scene::make_emitter(sound_shared_ptr_type apSound,
        insert_effect_collection_type aInserts)
    {
        return add_emitter(std::move(apSound), std::move(aInserts), {});
    }

    emitter_shared_ptr_type openal_scene::make_emitter(sound_shared_ptr_type apSound,
        std::span<const audio_vector3_type> aChannelPositions)
    {
        if (aChannelPositions.empty()) throw exception("an emitter placing a sound's channels needs a place for each");

        return add_emitter(std::move(apSound), {},
            std::vector<audio_vector3_type>(aChannelPositions.begin(), aChannelPositions.end()));
    }

    emitter_shared_ptr_type openal_scene::add_emitter(sound_shared_ptr_type apSound,
        insert_effect_collection_type aInserts, std::vector<audio_vector3_type> aChannelOffsets)
    {
        if (!apSound) throw exception("tried to make an emitter from a null sound");

        for (const auto &pInsert : aInserts)
            if (!pInsert) throw exception("an insert effect in the chain was null");

        const bool decodeUpFront = aInserts.empty() && aChannelOffsets.empty()
            && apSound->duration() < m_Policy.STREAMING_THRESHOLD_IN_SECONDS;

        std::shared_ptr<openal_emitter> pEmitter = decodeUpFront
            ? std::shared_ptr<openal_emitter>(new openal_simple_emitter(apSound, m_Policy))
            : std::shared_ptr<openal_emitter>(
                new openal_stream_emitter(apSound, m_Policy, std::move(aInserts), std::move(aChannelOffsets)));

        pEmitter->apply_listener(m_Listener);

        if (m_EffectSlot) pEmitter->route_to_send(m_EffectSlot.get());

        m_Emitters.push_back(pEmitter);

        return pEmitter;
    }

    emitter_group_shared_ptr_type openal_scene::make_emitter_group(std::span<const emitter_shared_ptr_type> aEmitters)
    {
        std::vector<std::shared_ptr<openal_emitter>> members;

        members.reserve(aEmitters.size());

        for (const auto &pEmitter : aEmitters)
        {
            auto pMember = std::dynamic_pointer_cast<openal_emitter>(pEmitter);

            const bool ours = pMember && std::any_of(m_Emitters.begin(), m_Emitters.end(),
                [&pMember](const auto &aHeld) { return aHeld.lock() == pMember; });

            if (!ours) throw exception("an emitter group can only be made of this scene's emitters");

            members.push_back(std::move(pMember));
        }

        auto pGroup = std::make_shared<openal_emitter_group>(std::move(members));

        m_Groups.push_back(pGroup);

        return pGroup;
    }

    void openal_scene::remove(const emitter_shared_ptr_type &aEmitter)
    {
        const auto found = std::find_if(m_Emitters.begin(), m_Emitters.end(),
            [&aEmitter](const auto &aHeld) { return aHeld.lock() == aEmitter; });

        if (found == m_Emitters.end()) return;

        if (auto pEmitter = found->lock()) pEmitter->stop();

        m_Emitters.erase(found);
    }

    std::size_t openal_scene::emitter_count() const
    {
        return static_cast<std::size_t>(std::count_if(m_Emitters.begin(), m_Emitters.end(),
            [](const auto &aHeld) { return !aHeld.expired(); }));
    }

    void openal_scene::set_reverb(const reverb_parameters &a)
    {
        const auto &efx = efx_or_throw();

        const auto effect = ensure_effect(EFX_EFFECT_REVERB);

        efx.alEffectf(effect, EFX_REVERB_DENSITY, a.density);
        efx.alEffectf(effect, EFX_REVERB_DIFFUSION, a.diffusion);
        efx.alEffectf(effect, EFX_REVERB_GAIN, a.gain);
        efx.alEffectf(effect, EFX_REVERB_GAINHF, a.high_frequency_gain);
        efx.alEffectf(effect, EFX_REVERB_DECAY_TIME, a.decay_seconds);
        efx.alEffectf(effect, EFX_REVERB_DECAY_HFRATIO, a.decay_high_frequency_ratio);
        efx.alEffectf(effect, EFX_REVERB_REFLECTIONS_GAIN, a.reflections_gain);
        efx.alEffectf(effect, EFX_REVERB_REFLECTIONS_DELAY, a.reflections_delay_seconds);
        efx.alEffectf(effect, EFX_REVERB_LATE_REVERB_GAIN, a.late_gain);
        efx.alEffectf(effect, EFX_REVERB_LATE_REVERB_DELAY, a.late_delay_seconds);
        efx.alEffectf(effect, EFX_REVERB_ROOM_ROLLOFF_FACTOR, a.room_rolloff_factor);

        bind_effect_to_slot();
    }

    void openal_scene::set_echo(const echo_parameters &a)
    {
        const auto &efx = efx_or_throw();

        const auto effect = ensure_effect(EFX_EFFECT_ECHO);

        efx.alEffectf(effect, EFX_ECHO_DELAY, a.delay_seconds);
        efx.alEffectf(effect, EFX_ECHO_LRDELAY, a.left_right_delay_seconds);
        efx.alEffectf(effect, EFX_ECHO_DAMPING, a.damping);
        efx.alEffectf(effect, EFX_ECHO_FEEDBACK, a.feedback);
        efx.alEffectf(effect, EFX_ECHO_SPREAD, a.spread);

        bind_effect_to_slot();
    }

    void openal_scene::clear_send_effect()
    {
        if (!m_EffectSlot) return;

        for (const auto &pEmitter : lock_and_prune(m_Emitters)) pEmitter->route_to_send(EFX_EFFECTSLOT_NULL);

        m_EffectSlot.reset();
        m_Effect.reset();
    }

    bool openal_scene::has_send_effect() const
    {
        return static_cast<bool>(m_EffectSlot);
    }

    ALuint openal_scene::effect_slot_handle() const
    {
        return m_EffectSlot.get();   
    }

    ALuint openal_scene::ensure_effect(const ALint aType)
    {
        const auto &efx = efx_or_throw();

        if (!m_EffectSlot)
        {
            m_EffectSlot.reset([&efx]()
            {
                ALuint handle;

                efx.alGenAuxiliaryEffectSlots(1, &handle);

                if (alGetError() != AL_NO_ERROR) throw exception(
                    "could not create an auxiliary effect slot. Slots are a scarce device resource "
                    "and this one has run out");

                return handle;
            }(),
            [&efx](const ALuint a) { efx.alDeleteAuxiliaryEffectSlots(1, &a); });
        }

        if (!m_Effect)
        {
            m_Effect.reset([&efx]()
            {
                ALuint handle;

                efx.alGenEffects(1, &handle);

                if (alGetError() != AL_NO_ERROR) throw exception("could not create an effect");

                return handle;
            }(),
            [&efx](const ALuint a) { efx.alDeleteEffects(1, &a); });
        }

        const auto effect = m_Effect.get();

        efx.alEffecti(effect, EFX_EFFECT_TYPE, aType);

        if (alGetError() != AL_NO_ERROR) throw exception(
            "the backend does not support this effect type");

        return effect;
    }

    void openal_scene::bind_effect_to_slot()
    {
        efx_or_throw().alAuxiliaryEffectSloti(m_EffectSlot.get(), EFX_EFFECTSLOT_EFFECT,
            static_cast<ALint>(m_Effect.get()));

        for (const auto &pEmitter : lock_and_prune(m_Emitters))
            pEmitter->route_to_send(m_EffectSlot.get());
    }

    void openal_scene::update()
    {
        for (const auto &pEmitter : lock_and_prune(m_Emitters))
        {
            pEmitter->apply_listener(m_Listener);
            pEmitter->update();
        }

        for (const auto &pGroup : lock_and_prune(m_Groups)) pGroup->update();
    }
}
