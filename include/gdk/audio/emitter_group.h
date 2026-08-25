// © Joseph Cameron - All Rights Reserved

#ifndef GDK_AUDIO_EMITTER_GROUP_H
#define GDK_AUDIO_EMITTER_GROUP_H

#include <gdk/audio/types.h>

namespace gdk::audio {
    /// \brief emitters that start, stop, pause and resume as one
    class emitter_group {
    public:
        /// \brief begins every member at once; a member already playing starts again
        virtual void play() = 0;

        virtual void pause() = 0;

        virtual void resume() = 0;

        virtual void stop() = 0;

        /// \brief whether any member is emitting
        [[nodiscard]] virtual bool is_playing() const = 0;

        [[nodiscard]] virtual bool is_paused() const = 0;

        /// \brief how many members are still alive
        [[nodiscard]] virtual std::size_t size() const = 0;

        virtual ~emitter_group() = default;
    };
}

#endif
