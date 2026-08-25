// © Joseph Cameron - All Rights Reserved

#ifndef GDK_AUDIO_EXCEPTION_H
#define GDK_AUDIO_EXCEPTION_H

#include <gdk/audio/types.h>

#include <exception>
#include <string>

namespace gdk::audio {
    /// \brief root exception type for this project
    class exception : public std::exception {
    public:
        exception() = default;

        exception(std::string aWhat);

        virtual ~exception() override = default;

        virtual const char *what() const noexcept override;

    private:
        std::string mWhat = "gdk::audio::exception";
    };
}

#endif
