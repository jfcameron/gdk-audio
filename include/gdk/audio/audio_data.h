// © 2020 Joseph Cameron - All Rights Reserved

#ifndef GDK_AUDIO_DATA_H
#define GDK_AUDIO_DATA_H

#include <memory>
#include <vector>

namespace gdk::audio
{
	/// \brief POD used to construct sound objects
	struct audio_data final
	{
	public:
		using buffer_data_type = std::vector<unsigned char>;

		using buffer_data_ptr_type = std::shared_ptr<buffer_data_type>;

		enum class encoding_type
		{
			vorbis,
			none
		};

	//private:
		encoding_type m_Encoding;

		std::shared_ptr<buffer_data_type> m_Data;

	public:
		decltype(m_Encoding) getEncoding() const;

		decltype(m_Data) getData() const;


		audio_data(encoding_type aEncoding, buffer_data_ptr_type aData);

		//todo: operators
	};
}

#endif
