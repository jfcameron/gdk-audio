// © Joseph Cameron - All Rights Reserved

#ifndef GDK_AUDIO_OPENAL_CONTEXT_H
#define GDK_AUDIO_OPENAL_CONTEXT_H

#include <gdk/audio/context.h>
#include <gdk/audio/openal_policy.h>
#include <gdk/audio/openal_scene.h>

#include <AL/al.h>
#include <AL/alc.h>

#include <functional>
#include <memory>
#include <vector>

namespace gdk::audio
{
    /// \brief openal implementation of context
    class openal_context final : public context
    {
    public:
        using device_pointer_type = std::unique_ptr<ALCdevice, std::function<void(ALCdevice *const)>>;
        using openal_context_pointer_type = std::unique_ptr<ALCcontext, std::function<void(ALCcontext *const)>>;

    private:
        openal_policy m_Policy;

        device_pointer_type m_pCurrentDevice;

        openal_context_pointer_type m_pContext;

    public:
        /// \brief opens the default audio device and makes a context current on it
        /// \warning throws if no device can be opened
        [[nodiscard]] static context_unique_ptr_type make(openal_policy aPolicy = {});

        [[nodiscard]] virtual scene_shared_ptr_type make_scene() override;

        [[nodiscard]] virtual std::vector<std::string> capture_device_names() const override;

        [[nodiscard]] virtual microphone_shared_ptr_type make_microphone(
            const microphone::request &aRequest) override;

        virtual ~openal_context() override;

    private:
        openal_context(openal_policy aPolicy);
    };
}

#endif

