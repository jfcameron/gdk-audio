// © 2019 Joseph Cameron - All Rights Reserved

#ifndef GDK_AUDIO_OPENAL_CONTEXT_H
#define GDK_AUDIO_OPENAL_CONTEXT_H

#include <gdk/audio/context.h>
#include <gdk/audio/openal_emitter.h>

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
        using emitter_collection_type = std::vector<std::shared_ptr<openal_emitter>>;

    private:
        device_pointer_type m_pCurrentDevice;

        openal_context_pointer_type m_pContext;

        emitter_collection_type m_Emitters;

    public:
        virtual std::shared_ptr<sound> makeSound(const std::string &filePath) override;

        virtual std::shared_ptr<sound> makeSound(std::vector<unsigned char> fileBuffer) override;

        virtual std::shared_ptr<emitter> makeEmitter(std::shared_ptr<sound> aSound) override;

        virtual void update() override;

        openal_context();
    };
}

#endif

