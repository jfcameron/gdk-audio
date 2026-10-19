// © Joseph Cameron - All Rights Reserved

#ifndef GDK_AUDIO_CONFORMANCE_H
#define GDK_AUDIO_CONFORMANCE_H

#include <gdk/audio/context.h>
#include <gdk/audio/types.h>

#include <string_view>

namespace gdk::audio::conformance {
    [[nodiscard]] context_shared_ptr_type make_context();

    [[nodiscard]] std::string_view implementation();
}

#endif
