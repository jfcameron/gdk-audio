// © Joseph Cameron - All Rights Reserved

#include <gdk/audio/null_scene.h>

#include <algorithm>

using namespace gdk::audio;

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
