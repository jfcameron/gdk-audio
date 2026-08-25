// © Joseph Cameron - All Rights Reserved

#include <gdk/audio/exception.h>

using namespace gdk::audio;

const char *exception::what() const noexcept {
    return mWhat.c_str();
}

exception::exception(std::string aWhat)
: mWhat(std::move(aWhat))
{}
