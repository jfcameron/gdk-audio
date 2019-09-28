// © 2019 Joseph Cameron - All Rights Reserved

#ifndef GDK_AUDIO_OPENAL_CONTEXT_H
#define GDK_AUDIO_OPENAL_CONTEXT_H

#include <gdk/audio/context.h>

#include <AL/al.h>
#include <AL/alc.h>

#include <functional>
#include <memory>

namespace gdk::audio
{
    /// \brief openal implementation of context
    class openal_context : public context
    {
        using device_pointer_type = std::unique_ptr<ALCdevice, std::function<void(ALCdevice *const)>>;
        using openal_context_pointer_type = std::unique_ptr<ALCcontext, std::function<void(ALCcontext *const)>>;

        device_pointer_type m_pCurrentDevice;

        openal_context_pointer_type m_pContext;

    public:
        openal_context();
    };
}

#endif

