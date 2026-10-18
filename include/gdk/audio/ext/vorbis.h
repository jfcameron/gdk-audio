// © Joseph Cameron - All Rights Reserved

#ifndef GDK_AUDIO_EXT_VORBIS_H
#define GDK_AUDIO_EXT_VORBIS_H

#include <gdk/audio/types.h>

#include <cstddef>
#include <vector>

namespace gdk::audio::ext {
    /// \brief creates a sound from audio data encoded in ogg vorbis. decodes on the fly
    [[nodiscard]] sound_shared_ptr_type make_vorbis_sound(std::vector<std::byte> aData);
}

#endif
