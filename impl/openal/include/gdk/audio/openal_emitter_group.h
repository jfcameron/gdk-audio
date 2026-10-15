// © Joseph Cameron - All Rights Reserved

#ifndef GDK_AUDIO_OPENAL_EMITTER_GROUP_H
#define GDK_AUDIO_OPENAL_EMITTER_GROUP_H

#include <gdk/audio/emitter_group.h>
#include <gdk/audio/openal_emitter.h>
#include <gdk/audio/types.h>

#include <AL/al.h>

#include <memory>
#include <vector>

namespace gdk::audio
{
    /// \brief openal implementation of emitter_group: one alSourcePlayv over every member's sources
    class openal_emitter_group final : public emitter_group
    {
        std::vector<std::weak_ptr<openal_emitter>> m_Members;

        [[nodiscard]] std::vector<std::shared_ptr<openal_emitter>> members() const;

        [[nodiscard]] std::vector<ALuint> sources() const;

    public:
        explicit openal_emitter_group(std::vector<std::shared_ptr<openal_emitter>> aMembers);

        virtual void play() override;

        virtual void pause() override;

        virtual void resume() override;

        virtual void stop() override;

        [[nodiscard]] virtual bool is_playing() const override;

        [[nodiscard]] virtual bool is_paused() const override;

        [[nodiscard]] virtual std::size_t size() const override;

        void update();

        virtual ~openal_emitter_group() override;
    };
}

#endif
