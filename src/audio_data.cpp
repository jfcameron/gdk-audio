// © 2020 Joseph Cameron - All Rights Reserved

#include <gdk/audio/audio_data.h>

using namespace gdk::audio;

audio_data::audio_data(encoding_type aEncoding, buffer_data_ptr_type aData)
	: m_Encoding(aEncoding)
	, m_Data(aData)
{}

decltype(audio_data::m_Encoding) audio_data::getEncoding() const
{
	return m_Encoding;
}

decltype(audio_data::m_Data) audio_data::getData() const
{
	return m_Data;
}