// © 2019 Joseph Cameron - All Rights Reserved

#ifndef GDK_AUDIO_OPENAL_SIMPLE_SOUND_H
#define GDK_AUDIO_OPENAL_SIMPLE_SOUND_H

#include <gdk/audio/openal_sound.h>

#include <jfc/memory/smart_handle.h>

#include <AL/al.h>
#include <AL/alc.h>

#include <vector>

namespace gdk::audio
{
    /// \brief single buffer sound impl
    /// less stateful than stream, can be shared among many emitters with no geometric ram cost (PCM buffers, decoder) but single pcm buffer will be very large
    class openal_simple_sound : public openal_sound
    {
    public:
        using file_buffer_type = std::vector<unsigned char>;

    private:
        jfc::memory::smart_handle<ALuint> m_ALBufferHandle;

    public:
        virtual std::vector<ALuint> getAlBufferHandles() override;

        virtual ~openal_simple_sound() = default;

        /// \brief constructs from a path to a file
        openal_simple_sound(const std::string &aFilePath);

        /// \brief constructs from an in-memory file buffer
        openal_simple_sound(const file_buffer_type &aFileBuffer);
    };
}

#endif

