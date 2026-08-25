// © Joseph Cameron - All Rights Reserved

#ifndef GDK_AUDIO_SEND_EFFECT_H
#define GDK_AUDIO_SEND_EFFECT_H

#include <gdk/audio/types.h>

/// \file the small closed set of send effects that the implementation provides.
///
/// A send effect (constrast with insert effect) is one applied to a sound being played back 
/// by an emitter after it has been mixed for spatialisation by the scene.
///
/// Unlike insert effects, there is currently no user-definable mechanism for send effects
namespace gdk::audio {
    struct reverb_parameters final {
        //! how closely spaced the reflections are. 0 is sparse and metallic, 1 is dense.
        audio_floating_point_type density = 1.0f;

        //! how quickly echoes smear together. 0 is a slap, 1 is a smooth tail.
        audio_floating_point_type diffusion = 1.0f;

        //! overall level of the effect
        audio_floating_point_type gain = 0.32f;

        //! level of the high frequencies, so a room can be made dull without being quiet
        audio_floating_point_type high_frequency_gain = 0.89f;

        //! seconds for the tail to fall away. A cathedral is several, a cupboard is a fraction.
        audio_floating_point_type decay_seconds = 1.49f;

        //! how much faster the high frequencies decay than the low. 1 is together.
        audio_floating_point_type decay_high_frequency_ratio = 0.83f;

        audio_floating_point_type reflections_gain = 0.05f;
        audio_floating_point_type reflections_delay_seconds = 0.007f;

        audio_floating_point_type late_gain = 1.26f;
        audio_floating_point_type late_delay_seconds = 0.011f;

        //! how much distance from the listener attenuates the reverb as well as the dry sound
        audio_floating_point_type room_rolloff_factor = 0.0f;
    };

    /// \brief a discrete repeat, distinct from reverb's wash
    struct echo_parameters final {
        //! seconds between repeats
        audio_floating_point_type delay_seconds = 0.1f;

        //! seconds between the left and right taps, which is what makes it feel wide
        audio_floating_point_type left_right_delay_seconds = 0.1f;

        //! how much high frequency each repeat loses, so repeats get duller rather than just quieter
        audio_floating_point_type damping = 0.5f;

        //! how much of a repeat is fed back in. 0 is a single echo.
        audio_floating_point_type feedback = 0.5f;

        //! how the repeats are placed across the stereo field. -1 and 1 are hard opposite.
        audio_floating_point_type spread = -1.0f;
    };
}

#endif
