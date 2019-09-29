// © 2019 Joseph Cameron - All Rights Reserved
// brainstorming the interface and raii layers for game centric audio library (3d scene etc)

//TODO think about hearing impaired support. likely gui would be subtitles with vectors pointing towards source. This would require getPosition, getMagntitude. Possibly getEffects, so you could subtitle "muffled" etc.

#include <gdk/audio/context.h>
#include <gdk/audio/emitter.h>
#include <gdk/audio/sound.h>
#include <gdk/audio/stb_vorbis.h>

#include <AL/al.h>
#include <AL/alc.h>

#include <array>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>

//TODO remove
#include <gdk/audio/openal_context.h>
#include <gdk/audio/openal_stream.h>
#include <gdk/audio/openal_emitter.h>

int main(int argc, char **argv)
{
    if (argc != 2) throw std::invalid_argument("program requires 1 paramter: path to an ogg vorbis file\n");

    auto pContext = std::unique_ptr<gdk::audio::context>(new gdk::audio::openal_context());

    {
        std::shared_ptr<gdk::audio::sound> pStream(new gdk::audio::openal_stream(argv[1]));//auto pStream = std::make_shared<gdk::audio::sound>(gdk::audio::openal_stream(argv[1]));

        auto pEmitter(std::unique_ptr<gdk::audio::emitter>(new gdk::audio::openal_emitter(std::dynamic_pointer_cast<gdk::audio::openal_stream>(pStream))));

        pEmitter->play();
    }

    return EXIT_SUCCESS;
}

