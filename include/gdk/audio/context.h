// © 2019 Joseph Cameron - All Rights Reserved

#ifndef GDK_AUDIO_CONTEXT_H
#define GDK_AUDIO_CONTEXT_H

#include <gdk/audio/sound.h>
#include <gdk/audio/emitter.h>

#include <functional>
#include <memory>

// TODO: non-static makes should return weak_ptrs, since emitted instances cannot exist outside of their context
// when a context falls out of scope, all its emissions should be cleaned. OpenAL is very stateful and requires this kind of collaboration to prevent leaks (cannot free bound buffer handles etc).
namespace gdk::audio
{
    /// \brief software connection to audio hardware, implementation global state goes here?
    /// active device etc?
    /// factories?
    class context
    {
    public:
        /// \brief used to specify implementation behind context::make
        enum class implementation
        {
            OpenAL
        };

        /// \brief factory method, creates a context of specified implementation
        static std::unique_ptr<context> make(implementation impl);

        /// \brief builds a sound from a filebuffer
        /// \warn throws if the buffer does not contain encoded audio data of a type the context supports
        virtual std::shared_ptr<sound> makeSound(sound::file_buffer_type fileBuffer) = 0;

        /// \brief builds a sound from a path to a file
        /// \warn throws if the buffer does not contain encoded audio data of a type the context supports
        virtual std::shared_ptr<sound> makeSound(const std::string &filePath) = 0;
      
        /// \brief builds an emitter from a pointer to a sound
        virtual std::shared_ptr<emitter> makeEmitter(std::shared_ptr<sound> aSound) = 0;

        /// \brief list devices (headphone, speaker, ...) by name
        //virtual std::vector<std::string> getDevices() = 0;

        /// \brief change the current device by name
        //virtual void setActiveDevice(const std::string &aDeviceName) = 0;

        /// \brief loop behaviour; must be called in your update loop for gdk::audio to behave correctly
        virtual void update() = 0;

        /// TODO: cleanup methods? In openal case cleanup is very stateful (cannot free buffers that are "in use" (attached to a source) etc.

        virtual ~context() = default;
    };
}

#endif

