// © 2019 Joseph Cameron - All Rights Reserved

#ifndef GDK_AUDIO_OPENAL_SOUND_H
#define GDK_AUDIO_OPENAL_SOUND_H

#include <gdk/audio/sound.h>

#include <AL/al.h>
#include <AL/alc.h>

#include <vector>

namespace gdk::audio
{
    /// \brief common interface for all openal sound implementations
    class openal_sound : public sound
    {
    public:
        using file_buffer_type = std::vector<unsigned char>;

        virtual std::vector<ALuint> getAlBufferHandles() = 0;       
    };
}

#endif

