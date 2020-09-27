// © 2019 Joseph Cameron - All Rights Reserved

#include <gdk/audio/openal_stream.h>

#include <iostream>
#include <fstream>

namespace gdk::audio
{
    std::vector<ALuint> openal_stream::getAlBufferHandles()
    {
        return {m_alBufferHandles.get().begin(), m_alBufferHandles.get().end()};
    }

    openal_stream::openal_stream(const std::string &aOggVorbisFileName) : openal_stream([&aOggVorbisFileName]()
    {
        std::ifstream in(aOggVorbisFileName, std::ios::in | std::ios::binary);

        if (!in.is_open()) throw std::invalid_argument("could not open file: " + aOggVorbisFileName);
        
        return std::make_shared<file_buffer_type>(std::vector<unsigned char>((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>()));
    }())
    {}

    openal_stream::openal_stream(const std::shared_ptr<file_buffer_type> aOggVorbisData)
    : m_pOggVorbisFileBuffer(aOggVorbisData)
    , m_pDecoder({
        [&]()
        {
            int error;
            stb_vorbis *vorbis = stb_vorbis_open_memory(&(*m_pOggVorbisFileBuffer.get())[0], m_pOggVorbisFileBuffer.get()->size(), &error, nullptr);

			//TODO: handle error enum in a more informative way. (pass along the specific error or at least decorate exception.what) 
            if (!vorbis || error != VORBIS__no_error) 
				throw std::invalid_argument("ogg vorbis data is badly formed; could not create decoder"); 

            return vorbis;
        }()
        ,
        [](stb_vorbis *const p)
        {
            stb_vorbis_close(p);
        }
    })
    , m_VorbisInfo(stb_vorbis_get_info(m_pDecoder.get()))
    , m_alBufferHandles([&]()
    {
        decltype(m_alBufferHandles)::handle_type newBufferHandles;

        alGenBuffers(newBufferHandles.size(), &newBufferHandles.front());

        const ALenum format = [](int channelCount)
        {
            switch(channelCount)
            {
                case 1: return AL_FORMAT_MONO16;
                case 2: return AL_FORMAT_STEREO16; // TODO: support 8s?
            }
            
            throw std::invalid_argument("unsupported channel count in ogg vorbis file");
        }(m_VorbisInfo.channels);

        //TODO clean this up
        m_Format = format;

        for (const auto &current_handle : newBufferHandles)
        {
            stb_vorbis_get_samples_short_interleaved(m_pDecoder.get()
                , m_VorbisInfo.channels
                , &m_PCMBuffer.front()
                , m_PCMBuffer.size());

            alBufferData(current_handle
                , format
                , &m_PCMBuffer.front()
                , m_PCMBuffer.size() * sizeof(pcm_buffer_type::value_type)
                , m_VorbisInfo.sample_rate);
        }

        return newBufferHandles;
    }(),
    [](const decltype(m_alBufferHandles)::handle_type a)
    {
        alDeleteBuffers(a.size(), &a.front());
    })
    {}

    bool openal_stream::decodeNextSamples(ALuint aOutputPCMBuffer)    
    {
		if (int amount = stb_vorbis_get_samples_short_interleaved(m_pDecoder.get(),
			m_VorbisInfo.channels,
			&m_PCMBuffer.front(),
			m_PCMBuffer.size()))
		{
			std::cout << "hello?\n";

			alBufferData(aOutputPCMBuffer
				, m_Format
				, &m_PCMBuffer.front()
				, m_PCMBuffer.size() * sizeof(pcm_buffer_type::value_type)
				, m_VorbisInfo.sample_rate);

			//stb_vorbis_seek_start(m_pDecoder.get());

			return true;
		}
		else
		{
			std::cout << "out of data\n";

			stb_vorbis_seek_start(m_pDecoder.get());

			//m_alBufferHandles.

			// This is where the issue is!
			m_alBufferHandles = { [&]()
			{
				decltype(m_alBufferHandles)::handle_type newBufferHandles;

				alGenBuffers(newBufferHandles.size(), &newBufferHandles.front());

				const ALenum format = [](int channelCount)
				{
					switch (channelCount)
					{
					case 1: return AL_FORMAT_MONO16;
					case 2: return AL_FORMAT_STEREO16; // TODO: support 8s?
					}

					throw std::invalid_argument("unsupported channel count in ogg vorbis file");
				}(m_VorbisInfo.channels);

				//TODO clean this up
				m_Format = format;

				for (const auto& current_handle : newBufferHandles)
				{
					stb_vorbis_get_samples_short_interleaved(m_pDecoder.get()
						, m_VorbisInfo.channels
						, &m_PCMBuffer.front()
						, m_PCMBuffer.size());

					alBufferData(current_handle
						, format
						, &m_PCMBuffer.front()
						, m_PCMBuffer.size() * sizeof(pcm_buffer_type::value_type)
						, m_VorbisInfo.sample_rate);
				}

				return newBufferHandles;
			}(),
				[](const decltype(m_alBufferHandles)::handle_type a)
			{
				alDeleteBuffers(a.size(), &a.front());
			}};
		}

        return false;
    }
}

