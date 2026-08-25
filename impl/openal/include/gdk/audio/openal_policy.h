// © Joseph Cameron - All Rights Reserved

#ifndef GDK_AUDIO_OPENAL_POLICY_H
#define GDK_AUDIO_OPENAL_POLICY_H

#include <gdk/audio/types.h>

#include <cstddef>

namespace gdk::audio {
    /// \brief how a sound's volume falls off with distance
    enum class distance_model {
        none, 
        inverse,
        inverse_clamped,
        linear,
        linear_clamped,
        exponent,
        exponent_clamped
    };

    /// \brief user-definable configuration for the implementation
    struct openal_policy final {
        /// \brief the attenuation curve every emitter is heard through
        const distance_model DISTANCE_MODEL{distance_model::linear_clamped};

        /// \brief how pronounced doppler shift is. 0 disables it, 1 is physical.
        const audio_floating_point_type DOPPLER_FACTOR{1};

        /// \brief propagation speed in world units per second, against which doppler is computed.
        /// The default is metres per second in dry air, so it assumes one unit is one metre.
        const audio_floating_point_type SPEED_OF_SOUND{343.3f};

        /// \brief a sound longer than this is streamed rather than decoded up front
        const audio_floating_point_type STREAMING_THRESHOLD_IN_SECONDS{5};

        /// \brief how much audio one streaming buffer holds
        const std::size_t STREAM_FRAMES_PER_BUFFER{4096};

        /// \brief how many buffers a streaming emitter cycles through. Two is the minimum that lets
        /// one play while the next is filled.
        const std::size_t STREAM_BUFFER_COUNT{2};

        /// \brief how much audio is pulled per read while decoding a sound up front
        const std::size_t DECODE_FRAMES_PER_READ{4096};
    };
}

#endif
