// © 2019 Joseph Cameron - All Rights Reserved

#ifndef GDK_AUDIO_OPENAL_SOUND_H
#define GDK_AUDIO_OPENAL_SOUND_H

#include <gdk/audio/sound.h>

#include <AL/al.h>
#include <AL/alc.h>

#include <vector>
#include <cstddef>
#include <type_traits>

namespace gdk::audio
{
    /// \brief data format for sound resource in openal implementation
    class openal_sound : public sound
    {
	private:
		encoding_type m_Encoding;

		/// \brief entire file copied to memory. can be shared between stream instances
		file_buffer_type m_pOggVorbisFileBuffer;

	public:
		decltype(m_Encoding) getEncoding() const;

		const decltype(m_pOggVorbisFileBuffer) &getData() const;


		/// \brief construct a sound from an encoding and file buffer
		openal_sound(decltype(m_Encoding) aEncoding,
			const decltype(m_pOggVorbisFileBuffer) aOggVorbisData);
    };
}

#endif

