// © Joseph Cameron - All Rights Reserved

#include <gdk/audio/context.h>
#include <gdk/audio/emitter.h>
#include <gdk/audio/listener.h>
#include <gdk/audio/openal_context.h>
#include <gdk/audio/openal_policy.h>
#include <gdk/audio/scene.h>
#include <gdk/audio/sound.h>

#include <gdk/math_constants.h>

#include <jfc/POL_chubby_cat_short.ogg.h>

#include <chrono>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <thread>

using namespace gdk;

namespace {
    constexpr float ORBIT_RADIUS = 6.0f;
    constexpr float FRAME_SECONDS = 1.0f / 60.0f;
    constexpr float WALK_AWAY_DISTANCE = 45.0f;

    [[nodiscard]] float lerp(const float a, const float b, const float t) {
        return a + (b - a) * t;
    }
}

int main() {
    const audio::openal_policy policy{
        .DISTANCE_MODEL = audio::distance_model::inverse_clamped,
        .DOPPLER_FACTOR = 1.0f,                 
        .SPEED_OF_SOUND = 343.3f,               
    };

    auto pAudio = audio::openal_context::make(policy);

    auto pScene = pAudio->make_scene();

    auto pSound = audio::make_vorbis_sound(POL_chubby_cat_short_ogg,
        sizeof(POL_chubby_cat_short_ogg));

    std::cout << "sound: " << pSound->channel_count() << " channel, "
        << pSound->sample_rate() << " Hz, " << pSound->duration() << "s\n";

    if (pSound->channel_count() != 1)
        std::cout << "  warning: not mono, so nothing below will be heard in space\n";

    auto pEmitter = pScene->make_emitter(pSound);
    auto &ears = pScene->get_listener();

    pEmitter->set_position({0, 0, 0});

    pEmitter->set_attenuation(1.0f, 40.0f, 1.0f);

    pEmitter->play();

    const float phase = pSound->duration() / 4.0f;

    int announced = -1;

    for (float elapsed = 0; pEmitter->is_playing(); elapsed += FRAME_SECONDS) {
        const int current = static_cast<int>(elapsed / phase);
        const float t = std::fmod(elapsed, phase) / phase;   

        if (current != announced) {
            static const char *const NAMES[] = {
                "1/4 orbit: the listener circles the emitter. Only the ears move.",
                "2/4 attenuation: the listener walks away and back. set_attenuation decides the curve.",
                "3/4 cone: the emitter now points one way. Outside the cone it drops to a quarter.",
                "4/4 gain: the emitter fades out, then the listener fades the whole scene out."
            };

            if (current < 4) std::cout << NAMES[current] << "\n";

            announced = current;
        }

        switch (current) {
            case 0: {
                const float angle = 2.0f * numbers::pi_f * t;

                ears.set_transform({std::sin(angle) * ORBIT_RADIUS, 0, std::cos(angle) * ORBIT_RADIUS},
                    audio::audio_quaternion_type::from_euler({0, angle, 0}));

                const float speed = 2.0f * numbers::pi_f * ORBIT_RADIUS / phase;

                ears.set_velocity({std::cos(angle) * speed, 0, -std::sin(angle) * speed});
            } break;

            case 1: {
                const float away = t < 0.5f ? t * 2 : (1 - t) * 2;

                ears.set_transform({0, 0, lerp(1.0f, WALK_AWAY_DISTANCE, away)},
                    audio::audio_quaternion_type::identity);

                ears.set_velocity(audio::audio_vector3_type::zero);
            } break;

            case 2: {
                pEmitter->set_direction({1, 0, 0});
                pEmitter->set_cone(numbers::pi_f / 4, numbers::pi_f / 2, 0.25f);

                const float angle = 2.0f * numbers::pi_f * t;

                ears.set_transform({std::sin(angle) * ORBIT_RADIUS, 0, std::cos(angle) * ORBIT_RADIUS},
                    audio::audio_quaternion_type::from_euler({0, angle, 0}));
            } break;

            case 3: {
                pEmitter->set_direction(audio::audio_vector3_type::zero);

                ears.set_transform({0, 0, 2}, audio::audio_quaternion_type::identity);

                pEmitter->set_gain(t < 0.5f ? lerp(1.0f, 0.25f, t * 2) : 0.25f);

                ears.set_gain(t < 0.5f ? 1.0f : lerp(1.0f, 0.0f, (t - 0.5f) * 2));
            } break;

            default: break;
        }

        pScene->update();

        std::this_thread::sleep_for(std::chrono::duration<float>(FRAME_SECONDS));
    }

    return EXIT_SUCCESS;
}
