// © Joseph Cameron - All Rights Reserved

#ifndef GDK_AUDIO_OPENAL_STREAM_EMITTER_H
#define GDK_AUDIO_OPENAL_STREAM_EMITTER_H

#include <gdk/audio/openal_emitter.h>
#include <gdk/audio/insert_effect.h>
#include <gdk/audio/openal_policy.h>
#include <gdk/audio/sound.h>

#include <jfc/shared_handle.h>

#include <AL/al.h>
#include <AL/alc.h>

#include <array>
#include <memory>
#include <vector>

namespace gdk::audio
{
	/// \brief a stream emitter decodes its sound as it plays, a buffer at a time
    class openal_stream_emitter : public openal_emitter
    {
		openal_policy m_Policy;

		insert_effect_collection_type m_Inserts;

		sound_shared_ptr_type m_pSound;

		std::unique_ptr<pcm_stream> m_pStream;

		std::vector<unsigned char> m_PCMBuffer;

		std::size_t m_BytesPerFrame;

		ALenum m_Format;

		ALsizei m_SampleRate;

		std::vector<jfc::shared_handle<ALuint>> m_alBufferHandles;

		bool decode_next_samples(ALuint aOutputPCMBuffer);

    public:
        openal_stream_emitter(const sound_shared_ptr_type &apSound,
            const openal_policy &aPolicy, insert_effect_collection_type aInserts = {});

        virtual void play() override;

		virtual void stop() override;

        virtual void update() override;
    };
}

#endif

