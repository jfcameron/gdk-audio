// © 2019 Joseph Cameron - All Rights Reserved

//TODO think about hearing impaired support. 
//likely gui would be subtitles with vectors pointing towards source. 
//This would require getPosition, getMagntitude. Possibly getEffects, so you could subtitle "muffled" etc.
//
// ^ gdk-text-renderer is now functional. 
// hearing impaired support would be a separate lib that sits on top of
// gdk-audio and gdk-text-renderer.

#include <gdk/audio/context.h>
#include <gdk/audio/emitter.h>
#include <gdk/audio/sound.h>
#include <gdk/audio/stb_vorbis.h>
#include <gdk/audio/audio_data.h>

#include <AL/al.h>
#include <AL/alc.h>

#include <cstdlib>
#include <stdexcept>
#include <string>
#include <fstream>
#include <memory>

static inline std::shared_ptr<std::vector<unsigned char>> loadFileToBuffer(const std::string& aFilePath)
{
	std::ifstream in(aFilePath, std::ios::in | std::ios::binary);

	if (!in.is_open()) throw std::invalid_argument("could not open file: " + aFilePath);

	return std::shared_ptr<std::vector<unsigned char>>(new std::vector<unsigned char>((
		std::istreambuf_iterator<char>(in)), 
		std::istreambuf_iterator<char>()));
}

int main(int argc, char **argv)
{
    if (argc != 3) throw std::invalid_argument(
		"program requires 1 paramter: path to a supported audio file type (ogg vorbis)\n");

    auto pContext = gdk::audio::context::make(gdk::audio::context::implementation::OpenAL);
    {
        auto pSound(pContext->make_sound(gdk::audio::audio_data(gdk::audio::audio_data::encoding_type::vorbis,
			loadFileToBuffer(std::string(argv[1])))));

		auto pSound2(pContext->make_sound(gdk::audio::audio_data(gdk::audio::audio_data::encoding_type::vorbis,
			loadFileToBuffer(std::string(argv[2])))));

		auto pEmitter(pContext->make_emitter(pSound));

		auto pEmitter2(pContext->make_emitter(pSound2));

		pEmitter->play();

		pEmitter2->play();

		for (int i(5); i;)
		{
			pContext->update();

			if (!pEmitter->isPlaying()) pEmitter->play();

			if (!pEmitter2->isPlaying()) pEmitter2->play();
		}
    }

    return EXIT_SUCCESS;
}
