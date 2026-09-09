// © Joseph Cameron - All Rights Reserved

#ifndef GDK_AUDIO_SCENE_H
#define GDK_AUDIO_SCENE_H

#include <gdk/audio/types.h>
#include <gdk/audio/insert_effect.h>
#include <gdk/audio/listener.h>
#include <gdk/audio/send_effect.h>
#include <gdk/audio/sound.h>

namespace gdk::audio {
    /// \brief 3d space where sounds are heard.
    /// a scene contains one listener and many emitters
    class scene {
    public:
        /// \brief the ears of this scene. Never null, and owned by the scene.
        [[nodiscard]] virtual listener &get_listener() = 0;

        [[nodiscard]] virtual const listener &get_listener() const = 0;

        /// \brief builds an emitter for a sound and adds it to this scene
        [[nodiscard]] virtual emitter_shared_ptr_type make_emitter(sound_shared_ptr_type aSound) = 0;

        /// \brief builds an emitter whose audio passes through aInserts, in the order given
        [[nodiscard]] virtual emitter_shared_ptr_type make_emitter(sound_shared_ptr_type aSound,
            insert_effect_collection_type aInserts) = 0;

        /// \brief takes an emitter out of the scene, stopping it
        virtual void remove(const emitter_shared_ptr_type &aEmitter) = 0;

        /// \brief how many of this scene's emitters are still alive
        [[nodiscard]] virtual std::size_t emitter_count() const = 0;

        /// \brief give this scene a reverb, heard by every emitter in it
        virtual void set_reverb(const reverb_parameters &aParameters) = 0;

        /// \brief give this scene an echo, heard by every emitter in it
        /// \warning throws if the backend has no auxiliary send available
        virtual void set_echo(const echo_parameters &aParameters) = 0;

        /// \brief remove whatever send effect this scene
        virtual void clear_send_effect() = 0;

        /// \brief whether this scene currently has one
        [[nodiscard]] virtual bool has_send_effect() const = 0;

        /// \brief must be called in your update loop for the scene to be heard correctly
        ///
        /// This is where each emitter is placed relative to the listener and where streaming emitters
        /// are refilled.
        virtual void update() = 0;

        virtual ~scene() = default;
    };
}

#endif
