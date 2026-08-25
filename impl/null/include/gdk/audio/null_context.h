// © Joseph Cameron - All Rights Reserved

#ifndef GDK_AUDIO_NULL_CONTEXT_H
#define GDK_AUDIO_NULL_CONTEXT_H

#include <gdk/audio/context.h>
#include <gdk/audio/null_microphone.h>
#include <gdk/audio/null_scene.h>

#include <memory>

namespace gdk::audio {
    /// \brief an audio context that talks to no hardware
    class null_context final : public context {
    public:
        [[nodiscard]] static std::shared_ptr<null_context> make();

        [[nodiscard]] virtual scene_shared_ptr_type make_scene() override;

        [[nodiscard]] virtual std::vector<std::string> capture_device_names() const override;

        [[nodiscard]] virtual microphone_shared_ptr_type make_microphone(
            const microphone::request &aRequest) override;

        //! what capture_device_names will report. Empty by default: a machine may have no microphone.
        void set_capture_device_names(std::vector<std::string> aNames);

        //! how many scenes this context has built
        [[nodiscard]] std::size_t scene_count() const;

    private:
        null_context() = default;

        std::size_t mSceneCount{0};
        std::vector<std::string> mCaptureDevices;
    };
}

#endif
