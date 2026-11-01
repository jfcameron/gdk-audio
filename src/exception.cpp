// © Joseph Cameron - MIT License

#include <gdk/audio/exception.h>

namespace gdk::audio {
    exception::exception()
    : exception("gdk::audio::exception")
    {}

    exception::exception(const std::string &aWhat)
    : mpWhat(std::make_shared<const std::string>(aWhat))
    {}

    const char *exception::what() const noexcept { return mpWhat->c_str(); }
}
