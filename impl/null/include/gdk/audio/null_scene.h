// © Joseph Cameron - All Rights Reserved

#ifndef GDK_AUDIO_NULL_SCENE_H
#define GDK_AUDIO_NULL_SCENE_H

#include <gdk/audio/null_emitter.h>
#include <gdk/audio/null_listener.h>
#include <gdk/audio/scene.h>

#include <memory>
#include <vector>

namespace gdk::audio {
    /// \brief a sound space that makes no sound
    class null_scene final : public scene {
    public:
        [[nodiscard]] virtual listener &get_listener() override;
        [[nodiscard]] virtual const listener &get_listener() const override;

        [[nodiscard]] virtual emitter_shared_ptr_type make_emitter(
            sound_shared_ptr_type aSound) override;

        [[nodiscard]] virtual emitter_shared_ptr_type make_emitter(sound_shared_ptr_type aSound,
            insert_effect_collection_type aInserts) override;

        virtual void remove(const emitter_shared_ptr_type &aEmitter) override;

        [[nodiscard]] virtual std::size_t emitter_count() const override;

        virtual void set_reverb(const reverb_parameters &aParameters) override;
        virtual void set_echo(const echo_parameters &aParameters) override;
        virtual void clear_send_effect() override;
        [[nodiscard]] virtual bool has_send_effect() const override;

        virtual void update() override;

        //! how many times update has been called, so a caller's loop can be checked
        [[nodiscard]] std::size_t update_count() const;

    private:
        //! drops what has expired, which is what makes emitter_count honest
        void compact() const;

        null_listener mListener;

        mutable std::vector<std::weak_ptr<emitter>> mEmitters;

        bool mHasSendEffect{false};
        std::size_t mUpdateCount{0};
    };
}

#endif
