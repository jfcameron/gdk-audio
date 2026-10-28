// © Joseph Cameron - MIT License

#include "conformance.h"

#include <gdk/audio/null_context.h>

namespace gdk::audio::conformance {
    context_shared_ptr_type make_context() { return null_context::make(); }

    std::string_view implementation() { return "null"; }
}
