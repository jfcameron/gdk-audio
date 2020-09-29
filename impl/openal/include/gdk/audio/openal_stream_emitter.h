// © 2020 Joseph Cameron - All Rights Reserved

#ifndef GDK_AUDIO_OPENAL_STREAM_EMITTER_H
#define GDK_AUDIO_OPENAL_STREAM_EMITTER_H

#include <gdk/audio/openal_emitter.h>
#include <gdk/audio/openal_sound.h>
#include <gdk/audio/stb_vorbis.h>

#include <jfc/shared_handle.h>

#include <AL/al.h>
#include <AL/alc.h>

#include <array>
#include <vector>
#include <memory>

namespace gdk::audio
{
	/// \brief a stream emitter decodes the provided sound on the fly.
    class openal_stream_emitter : public openal_emitter
    {
	public:
		using vorbis_decoder_pointer_type = std::unique_ptr<stb_vorbis, std::function<void(stb_vorbis* const)>>;

		/// \brief size of the pcm buffer
		using pcm_buffer_type = std::array<short, 8192>;

	private:
		/// \brief ptr to the encoded data, from which audio is streamed
		std::shared_ptr<openal_sound> m_pSound;

		/// \brief decodes vorbis data to the PCM buffer
		vorbis_decoder_pointer_type m_pDecoder;

		/// \brief contains metadata to do with vorbis file (channel count, legnth in seconds, etc.)
		stb_vorbis_info m_VorbisInfo;

		/// \brief memory where decoded audio is briefly held before uploading to an al buffer handle
		pcm_buffer_type m_PCMBuffer = {};

		/// \brief channel & mono/stero
		ALenum m_Format = -1;

		/// \brief handles to the albuffers the PCM data is uploaded to
		jfc::shared_handle<std::array<ALuint, 2>> m_alBufferHandles;

		bool decodeNextSamples(ALuint aOutputPCMBuffer);

    public:
        openal_stream_emitter(std::shared_ptr<openal_sound> pSimpleSound);

        virtual void play() override;
        
		virtual void stop() override;

        virtual void update() override;
    };
}

#endif
