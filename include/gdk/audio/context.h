// © 2019 Joseph Cameron - All Rights Reserved

#ifndef GDK_AUDIO_CONTEXT_H
#define GDK_AUDIO_CONTEXT_H

#include <memory>

namespace gdk::audio
{
    /// \brief software connection to audio hardware, implementation global state goes here?
    /// active device etc?
    /// factories?
    class context
    {
    public:
        /// \brief can be used to specify implementation behind context::make
        enum class implementation
        {
            OpenAL
        };

        static std::unique_ptr<context> make(implementation impl);

        /// \brief list devices (headphone, speaker, ...) by name
        //virtual std::vector<std::string> getDevices() = 0;

        /// \brief change the current device by name
        //virtual void setActiveDevice(const std::string &aDeviceName) = 0;

        /// \brief factory methods for audio types; implementation dependenent types, hidden behind interface pointers

        /// \brief factory method for context: user specifies implementation, gets a pContext

        virtual ~context() = default;
    };
}

#endif

