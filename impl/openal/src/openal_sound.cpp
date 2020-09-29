// © 2020 Joseph Cameron - All Rights Reserved

#include <gdk/audio/openal_sound.h>

using namespace gdk::audio;

openal_sound::openal_sound(decltype(m_Encoding) aEncoding,
	const file_buffer_type aOggVorbisData)
	: m_Encoding(aEncoding)
	, m_pOggVorbisFileBuffer(aOggVorbisData)
{}


decltype(openal_sound::m_Encoding) openal_sound::getEncoding() const
{
	return m_Encoding;
}

const decltype(openal_sound::m_pOggVorbisFileBuffer)& openal_sound::getData() const
{
	return m_pOggVorbisFileBuffer;
}
