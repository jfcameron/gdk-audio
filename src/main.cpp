// © 2019 Joseph Cameron - All Rights Reserved
// brainstorming the interface and raii layers for game centric audio library (3d scene etc)

//TODO think about hearing impaired support. likely gui would be subtitles with vectors pointing towards source. This would require getPosition, getMagntitude. Possibly getEffects, so you could subtitle "muffled" etc.

#include <gdk/audio/context.h>
#include <gdk/audio/emitter.h>
#include <gdk/audio/sound.h>
#include <gdk/audio/stb_vorbis.h>

#include <AL/al.h>
#include <AL/alc.h>

#include <cstdlib>
#include <stdexcept>
#include <string>

int main(int argc, char **argv)
{
    if (argc != 2) throw std::invalid_argument("program requires 1 paramter: path to a supported audio file type (ogg vorbis)\n");

    auto pContext = gdk::audio::context::make(gdk::audio::context::implementation::OpenAL);
    {
        auto pSound(pContext->makeSound(std::string(argv[1])));
        {
            auto pEmitter(pContext->makeEmitter(pSound));

            pEmitter->play();

            // This is our "game update loop". In this case, the game will quit our emitter has stopped emitting.
            while (pEmitter->isPlaying())
            {
                pContext->update();
            }
        }
    }

    return EXIT_SUCCESS;
}

