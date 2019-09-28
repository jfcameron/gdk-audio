// if a scene contains a listener, play. if a scene does not have a player, do not play.
// should multiple listers exist? how exactly owuld that wokr? it would just be noise.

#include <gdk/audio/context.h>
#include <gdk/audio/emitter.h>
#include <gdk/audio/scene.h>
#include <gdk/audio/sound.h>

#include <vector>

int main(int argc, char **argv)
{
    using gdk;

    auto pContext = audio::context::make(audio::context::implementation::OpenAL); // specifying the implementation.

    auto pSound = pContext->makeSound("myCoolOggVorbisFile"); //sound is a resource type.

    std::vector<std::shared_ptr<audio::scene>> scenes;

    scenes.push_back(std::make_shared<audio::scene>(pContext->makeScene()); // returns a uniqueptr, this usecase demonstrates promotion to smartpointer, so same instance can be consumed multiple downstream palces.

    auto pEmitter = scenes.front()->makeEmitter(pSound);

    while (true)
    {
        if (keydown "A") pEmitter->emit();

        for (auto &scene : scenes)
        {
            scene->update();
        }
    }
}

