// © Joseph Cameron - All Rights Reserved

#include <gdk/audio/exception.h>
#include <gdk/audio/null_scene.h>

#include <algorithm>
#include <string>

using namespace gdk::audio;

null_emitter_group::null_emitter_group(std::vector<std::weak_ptr<emitter>> aMembers)
: mMembers(std::move(aMembers)) {}

std::vector<std::shared_ptr<emitter>> null_emitter_group::members() const {
    std::vector<std::shared_ptr<emitter>> out;

    for (const auto &member : mMembers) if (auto pMember = member.lock()) out.push_back(std::move(pMember));

    return out;
}

void null_emitter_group::play() { for (const auto &pMember : members()) pMember->play(); }

void null_emitter_group::pause() { for (const auto &pMember : members()) pMember->pause(); }

void null_emitter_group::resume() { for (const auto &pMember : members()) pMember->resume(); }

void null_emitter_group::stop() { for (const auto &pMember : members()) pMember->stop(); }

bool null_emitter_group::is_playing() const {
    for (const auto &pMember : members()) if (pMember->is_playing()) return true;

    return false;
}

bool null_emitter_group::is_paused() const {
    const auto living = members();

    return !living.empty() && std::all_of(living.begin(), living.end(),
        [](const auto &pMember) { return pMember->is_paused(); });
}

std::size_t null_emitter_group::size() const { return members().size(); }

listener &null_scene::get_listener() { return mListener; }

const listener &null_scene::get_listener() const { return mListener; }

void null_scene::compact() const {
    mEmitters.erase(std::remove_if(mEmitters.begin(), mEmitters.end(),
        [](const std::weak_ptr<emitter> &aEmitter) { return aEmitter.expired(); }), mEmitters.end());
}

emitter_shared_ptr_type null_scene::make_emitter(sound_shared_ptr_type) {
    auto pEmitter = std::make_shared<null_emitter>();

    mEmitters.push_back(pEmitter);

    return pEmitter;
}

emitter_shared_ptr_type null_scene::make_emitter(sound_shared_ptr_type aSound,
    insert_effect_collection_type) {
    return make_emitter(std::move(aSound));
}

emitter_shared_ptr_type null_scene::make_emitter(sound_shared_ptr_type aSound,
    std::span<const audio_vector3_type> aChannelPositions) {
    if (!aSound) throw exception("tried to make an emitter from a null sound");

    if (aChannelPositions.size() != aSound->channel_count())
        throw exception("an emitter placing a sound's channels needs a place for each: the sound has "
            + std::to_string(aSound->channel_count()) + " channels, and "
            + std::to_string(aChannelPositions.size()) + " places were given");

    auto pEmitter = std::make_shared<null_emitter>(
        std::vector<audio_vector3_type>(aChannelPositions.begin(), aChannelPositions.end()));

    mEmitters.push_back(pEmitter);

    return pEmitter;
}

emitter_group_shared_ptr_type null_scene::make_emitter_group(std::span<const emitter_shared_ptr_type> aEmitters) {
    std::vector<std::weak_ptr<emitter>> members;

    for (const auto &pEmitter : aEmitters) {
        const bool ours = pEmitter && std::any_of(mEmitters.begin(), mEmitters.end(),
            [&pEmitter](const auto &aHeld) { return aHeld.lock() == pEmitter; });

        if (!ours) throw exception("an emitter group can only be made of this scene's emitters");

        members.push_back(pEmitter);
    }

    return std::make_shared<null_emitter_group>(std::move(members));
}

void null_scene::remove(const emitter_shared_ptr_type &aEmitter) {
    if (aEmitter) aEmitter->stop();

    mEmitters.erase(std::remove_if(mEmitters.begin(), mEmitters.end(),
        [&aEmitter](const std::weak_ptr<emitter> &aHeld) {
            return aHeld.expired() || aHeld.lock() == aEmitter;
        }), mEmitters.end());
}

std::size_t null_scene::emitter_count() const {
    compact();

    return mEmitters.size();
}

void null_scene::set_reverb(const reverb_parameters &) { mHasSendEffect = true; }

void null_scene::set_echo(const echo_parameters &) { mHasSendEffect = true; }

void null_scene::clear_send_effect() { mHasSendEffect = false; }

bool null_scene::has_send_effect() const { return mHasSendEffect; }

void null_scene::update() { ++mUpdateCount; }

std::size_t null_scene::update_count() const { return mUpdateCount; }
