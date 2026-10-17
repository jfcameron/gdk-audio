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
#include <cstddef>
#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>

namespace gdk::audio
{
	/// \brief a stream emitter decodes its sound as it plays
    class openal_stream_emitter : public openal_emitter
    {
		openal_policy m_Policy;

		insert_effect_collection_type m_Inserts;

		sound_shared_ptr_type m_pSound;

		std::unique_ptr<pcm_stream> m_pStream;

		std::vector<unsigned char> m_PCMBuffer;

		std::vector<unsigned char> m_ChannelBuffer;

		std::size_t m_BytesPerFrame;

		std::size_t m_BytesPerSample;

		ALenum m_Format;

		ALsizei m_SampleRate;

		std::vector<std::vector<jfc::shared_handle<ALuint>>> m_alBufferHandles;

		std::unordered_map<ALuint, std::size_t> m_SlotOfBuffer;

		std::vector<std::size_t> m_FramesInSlot;

		std::size_t m_FramesRetired = 0;

		bool decode_next_samples(const std::size_t aSlot);

		void queue_from_stream();

		void unqueue_all();

		void refill();

		[[nodiscard]] bool is_split() const;

    protected:
        virtual void before_pause() override;

    public:
        openal_stream_emitter(const sound_shared_ptr_type &apSound,
            const openal_policy &aPolicy, insert_effect_collection_type aInserts = {},
            std::vector<vector_type> aChannelOffsets = {});

        virtual void play() override;

		virtual void stop() override;

		virtual void discard_queued() override;

		[[nodiscard]] virtual std::optional<double> playback_position() const override;

		virtual void begin_play() override;

        virtual void update() override;
    };
}

#endif
