// © Joseph Cameron - All Rights Reserved

#ifndef GDK_AUDIO_CONTEXT_H
#define GDK_AUDIO_CONTEXT_H

#include <gdk/audio/types.h>
#include <gdk/audio/microphone.h>
#include <gdk/audio/scene.h>
#include <gdk/audio/sound.h>

#include <cstddef>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace gdk::audio
{
    class context
    {
    public:
        /// \brief builds a sound space
        [[nodiscard]] virtual scene_shared_ptr_type make_scene() = 0;

        /// \brief the input devices this context can capture from
        [[nodiscard]] virtual std::vector<std::string> capture_device_names() const = 0;

        /// \brief open an input device and capture from it
        [[nodiscard]] virtual microphone_shared_ptr_type make_microphone(
            const microphone::request &aRequest) = 0;

        virtual ~context() = default;
    };
}

#endif

