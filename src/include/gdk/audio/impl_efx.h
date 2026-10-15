// © Joseph Cameron - All Rights Reserved

#ifndef GDK_AUDIO_IMPL_EFX_H
#define GDK_AUDIO_IMPL_EFX_H

#include <AL/al.h>

#ifndef AL_APIENTRY
#define AL_APIENTRY
#endif

namespace gdk::audio {
    struct efx_functions final {
        void (AL_APIENTRY *alGenEffects)(ALsizei, ALuint *);
        void (AL_APIENTRY *alDeleteEffects)(ALsizei, const ALuint *);
        void (AL_APIENTRY *alEffecti)(ALuint, ALenum, ALint);
        void (AL_APIENTRY *alEffectf)(ALuint, ALenum, ALfloat);

        void (AL_APIENTRY *alGenFilters)(ALsizei, ALuint *);
        void (AL_APIENTRY *alDeleteFilters)(ALsizei, const ALuint *);
        void (AL_APIENTRY *alFilteri)(ALuint, ALenum, ALint);
        void (AL_APIENTRY *alFilterf)(ALuint, ALenum, ALfloat);

        void (AL_APIENTRY *alGenAuxiliaryEffectSlots)(ALsizei, ALuint *);
        void (AL_APIENTRY *alDeleteAuxiliaryEffectSlots)(ALsizei, const ALuint *);
        void (AL_APIENTRY *alAuxiliaryEffectSloti)(ALuint, ALenum, ALint);
    };

    constexpr ALenum EFX_AUXILIARY_SEND_FILTER = 0x20006;
    constexpr ALenum EFX_DIRECT_FILTER = 0x20005;
    constexpr ALenum EFX_ECHO_DAMPING = 0x0003;
    constexpr ALenum EFX_ECHO_DELAY = 0x0001;
    constexpr ALenum EFX_ECHO_FEEDBACK = 0x0004;
    constexpr ALenum EFX_ECHO_LRDELAY = 0x0002;
    constexpr ALenum EFX_ECHO_SPREAD = 0x0005;
    constexpr ALenum EFX_EFFECTSLOT_EFFECT = 0x0001;
    constexpr ALuint EFX_EFFECTSLOT_NULL = 0x0000;
    constexpr ALint EFX_EFFECT_ECHO = 0x0004;
    constexpr ALint EFX_EFFECT_REVERB = 0x0001;
    constexpr ALenum EFX_EFFECT_TYPE = 0x8001;
    constexpr ALint EFX_FILTER_LOWPASS = 0x0001;
    constexpr ALuint EFX_FILTER_NULL = 0x0000;
    constexpr ALenum EFX_FILTER_TYPE = 0x8001;
    constexpr ALenum EFX_LOWPASS_GAIN = 0x0001;
    constexpr ALenum EFX_LOWPASS_GAINHF = 0x0002;
    constexpr ALenum EFX_REVERB_DECAY_HFRATIO = 0x0006;
    constexpr ALenum EFX_REVERB_DECAY_TIME = 0x0005;
    constexpr ALenum EFX_REVERB_DENSITY = 0x0001;
    constexpr ALenum EFX_REVERB_DIFFUSION = 0x0002;
    constexpr ALenum EFX_REVERB_GAIN = 0x0003;
    constexpr ALenum EFX_REVERB_GAINHF = 0x0004;
    constexpr ALenum EFX_REVERB_LATE_REVERB_DELAY = 0x000A;
    constexpr ALenum EFX_REVERB_LATE_REVERB_GAIN = 0x0009;
    constexpr ALenum EFX_REVERB_REFLECTIONS_DELAY = 0x0008;
    constexpr ALenum EFX_REVERB_REFLECTIONS_GAIN = 0x0007;
    constexpr ALenum EFX_REVERB_ROOM_ROLLOFF_FACTOR = 0x000C;

    [[nodiscard]] const efx_functions *efx();

    void forget_efx();
}

#endif
