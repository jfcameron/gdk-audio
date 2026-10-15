// © Joseph Cameron - All Rights Reserved

#ifndef GDK_AUDIO_NULL_SCENE_H
#define GDK_AUDIO_NULL_SCENE_H

#include <gdk/audio/emitter_group.h>
#include <gdk/audio/null_emitter.h>
#include <gdk/audio/null_listener.h>
#include <gdk/audio/scene.h>

#include <memory>
#include <span>
#include <vector>

namespace gdk::audio {
    /// \brief emitters that make no sound, as one
    class null_emitter_group final : public emitter_group {
    public:
        explicit null_emitter_group(std::vector<std::weak_ptr<emitter>> aMembers);

        virtual void play() override;
        virtual void pause() override;
        virtual void resume() override;
        virtual void stop() override;
        [[nodiscard]] virtual bool is_playing() const override;
        [[nodiscard]] virtual bool is_paused() const override;
        [[nodiscard]] virtual std::size_t size() const override;

    private:
        [[nodiscard]] std::vector<std::shared_ptr<emitter>> members() const;

        std::vector<std::weak_ptr<emitter>> mMembers;
    };

    /// \brief a sound space that makes no sound
    class null_scene final : public scene {
    public:
        [[nodiscard]] virtual listener &get_listener() override;
        [[nodiscard]] virtual const listener &get_listener() const override;

        [[nodiscard]] virtual emitter_shared_ptr_type make_emitter(
            sound_shared_ptr_type aSound) override;

        [[nodiscard]] virtual emitter_shared_ptr_type make_emitter(sound_shared_ptr_type aSound,
            insert_effect_collection_type aInserts) override;

        [[nodiscard]] virtual emitter_shared_ptr_type make_emitter(sound_shared_ptr_type aSound,
            std::span<const audio_vector3_type> aChannelPositions) override;

        [[nodiscard]] virtual emitter_group_shared_ptr_type make_emitter_group(
            std::span<const emitter_shared_ptr_type> aEmitters) override;

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
