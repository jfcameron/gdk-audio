// © 2019 Joseph Cameron - All Rights Reserved

#ifndef GDK_AUDIO_OPENAL_STREAM_EMITTER_H
#define GDK_AUDIO_OPENAL_STREAM_EMITTER_H

#include <gdk/audio/openal_emitter.h>
#include <gdk/audio/openal_stream.h>

#include <jfc/memory/smart_handle.h>

#include <AL/al.h>
#include <AL/alc.h>

#include <memory>

namespace gdk::audio
{
    class openal_stream_emitter : public openal_emitter
    {
        std::shared_ptr<openal_stream> m_pStream;

    public:
        openal_stream_emitter(std::shared_ptr<openal_stream> pSimpleSound);

        virtual void play() override;
        
        virtual void update() override;
    };
}

#endif

