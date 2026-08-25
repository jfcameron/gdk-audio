// © Joseph Cameron - All Rights Reserved

#include <gdk/audio/exception.h>
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
        return make_emitter(std::move(apSound), {});
    }

    emitter_shared_ptr_type openal_scene::make_emitter(sound_shared_ptr_type apSound,
        insert_effect_collection_type aInserts)
    {
        if (!apSound) throw exception("tried to make an emitter from a null sound");

        for (const auto &pInsert : aInserts)
            if (!pInsert) throw exception("an insert effect in the chain was null");

        const bool decodeUpFront = aInserts.empty()
            && apSound->duration() < m_Policy.STREAMING_THRESHOLD_IN_SECONDS;

        std::shared_ptr<openal_emitter> pEmitter = decodeUpFront
            ? std::shared_ptr<openal_emitter>(new openal_simple_emitter(apSound, m_Policy))
            : std::shared_ptr<openal_emitter>(
                new openal_stream_emitter(apSound, m_Policy, std::move(aInserts)));

        pEmitter->apply_listener(m_Listener);

        if (m_pEffectSlot) pEmitter->route_to_send(m_pEffectSlot->get());

        m_Emitters.push_back(pEmitter);

        return pEmitter;
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
        const auto effect = ensure_effect(AL_EFFECT_REVERB);

        alEffectf(effect, AL_REVERB_DENSITY, a.density);
        alEffectf(effect, AL_REVERB_DIFFUSION, a.diffusion);
        alEffectf(effect, AL_REVERB_GAIN, a.gain);
        alEffectf(effect, AL_REVERB_GAINHF, a.high_frequency_gain);
        alEffectf(effect, AL_REVERB_DECAY_TIME, a.decay_seconds);
        alEffectf(effect, AL_REVERB_DECAY_HFRATIO, a.decay_high_frequency_ratio);
        alEffectf(effect, AL_REVERB_REFLECTIONS_GAIN, a.reflections_gain);
        alEffectf(effect, AL_REVERB_REFLECTIONS_DELAY, a.reflections_delay_seconds);
        alEffectf(effect, AL_REVERB_LATE_REVERB_GAIN, a.late_gain);
        alEffectf(effect, AL_REVERB_LATE_REVERB_DELAY, a.late_delay_seconds);
        alEffectf(effect, AL_REVERB_ROOM_ROLLOFF_FACTOR, a.room_rolloff_factor);

        bind_effect_to_slot();
    }

    void openal_scene::set_echo(const echo_parameters &a)
    {
        const auto effect = ensure_effect(AL_EFFECT_ECHO);

        alEffectf(effect, AL_ECHO_DELAY, a.delay_seconds);
        alEffectf(effect, AL_ECHO_LRDELAY, a.left_right_delay_seconds);
        alEffectf(effect, AL_ECHO_DAMPING, a.damping);
        alEffectf(effect, AL_ECHO_FEEDBACK, a.feedback);
        alEffectf(effect, AL_ECHO_SPREAD, a.spread);

        bind_effect_to_slot();
    }

    void openal_scene::clear_send_effect()
    {
        if (!m_pEffectSlot) return;

        for (const auto &pEmitter : lock_and_prune(m_Emitters)) pEmitter->route_to_send(AL_EFFECTSLOT_NULL);

        m_pEffectSlot.reset();
        m_pEffect.reset();
    }

    bool openal_scene::has_send_effect() const
    {
        return static_cast<bool>(m_pEffectSlot);
    }

    ALuint openal_scene::effect_slot_handle() const
    {
        return m_pEffectSlot ? m_pEffectSlot->get() : static_cast<ALuint>(AL_EFFECTSLOT_NULL);
    }

    ALuint openal_scene::ensure_effect(const ALenum aType)
    {
        if (!m_pEffectSlot)
        {
            m_pEffectSlot.reset(new jfc::shared_handle<ALuint>([]()
            {
                ALuint handle;

                alGenAuxiliaryEffectSlots(1, &handle);

                if (alGetError() != AL_NO_ERROR) throw exception(
                    "could not create an auxiliary effect slot. Slots are a scarce device resource "
                    "and this one has run out");

                return handle;
            }(),
            [](const ALuint a) { alDeleteAuxiliaryEffectSlots(1, &a); }));
        }

        if (!m_pEffect)
        {
            m_pEffect.reset(new jfc::shared_handle<ALuint>([]()
            {
                ALuint handle;

                alGenEffects(1, &handle);

                if (alGetError() != AL_NO_ERROR) throw exception("could not create an effect");

                return handle;
            }(),
            [](const ALuint a) { alDeleteEffects(1, &a); }));
        }

        const auto effect = m_pEffect->get();

        alEffecti(effect, AL_EFFECT_TYPE, aType);

        if (alGetError() != AL_NO_ERROR) throw exception(
            "the backend does not support this effect type");

        return effect;
    }

    void openal_scene::bind_effect_to_slot()
    {
        alAuxiliaryEffectSloti(m_pEffectSlot->get(), AL_EFFECTSLOT_EFFECT,
            static_cast<ALint>(m_pEffect->get()));

        for (const auto &pEmitter : lock_and_prune(m_Emitters))
            pEmitter->route_to_send(m_pEffectSlot->get());
    }

    void openal_scene::update()
    {
        for (const auto &pEmitter : lock_and_prune(m_Emitters))
        {
            pEmitter->apply_listener(m_Listener);
            pEmitter->update();
        }
    }
}
