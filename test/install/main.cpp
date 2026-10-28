// © Joseph Cameron - MIT License

#include <gdk/audio/exception.h>
#include <gdk/audio/ext/vorbis.h>
#include <gdk/audio/null_context.h>

#include <cstdint>

int main() {
    using namespace gdk::audio;

    const std::int16_t silence[4] = {};

    const auto pScene = null_context::make()->make_scene();
    const auto pEmitter = pScene->make_emitter(make_pcm_sound(silence, 4, 1, 8000, pcm_format::signed_16));

    pEmitter->set_position({1, 2, 3});
    pEmitter->play();
    pScene->update();

    try {
        static_cast<void>(ext::make_vorbis_sound({}));

        return 1;
    }
    catch (const exception &) {}

    return pEmitter->is_playing() ? 0 : 1;
}
