// © 2019 Joseph Cameron - All Rights Reserved

#ifndef GDK_AUDIO_SOUND_H
#define GDK_AUDIO_SOUND_H

#include <vector>

namespace gdk::audio
{
    /// \brief user-facing interface to a sound
    /// sound is the resource interface type, represents a playable piece of audio. 
    /// generally abstraction of one or many buffers containing decoded audio data (PCM (pulse-code modulated audio)), either fully decoded or streamed.
    class sound
    {
    public:
        using file_buffer_type = std::vector<unsigned char>;
        //gettimeInSeconds

        //getchannelCount(mono, stero)

        virtual ~sound() = default;
    };
}

#endif

