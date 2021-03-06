// � 2020 Joseph Cameron - All Rights Reserved

#include <chrono>
#include <cstdlib>
#include <ctime> 
#include <iostream>
#include <iostream>
#include <math.h>
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <thread>

#include <gdk/audio/context.h>

#include <jfc/POL_chubby_cat_short.ogg.h>

using namespace gdk;

int main(int count, char** params)
{
    // Init the context, request openal implementation
    auto pAudio = audio::context::make(audio::context::implementation::openal);

    // load a vorbis file from buffer into an audio::sound.
    auto pSound = pAudio->make_sound(audio::sound::encoding_type::vorbis, std::vector<unsigned char>(
                POL_chubby_cat_short_ogg, POL_chubby_cat_short_ogg + sizeof(POL_chubby_cat_short_ogg) / sizeof(POL_chubby_cat_short_ogg[0])));

    // create an emitter, to emit the sound
    auto pEmitter = pAudio->make_emitter(pSound);

    pEmitter->set_pitch(0.9f);

    pEmitter->play();

    while (pEmitter->isPlaying()) pAudio->update();

    return EXIT_SUCCESS;
}

