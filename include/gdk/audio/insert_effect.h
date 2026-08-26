// © Joseph Cameron - All Rights Reserved

#ifndef GDK_AUDIO_INSERT_EFFECT_H
#define GDK_AUDIO_INSERT_EFFECT_H

#include <gdk/audio/types.h>
#include <gdk/audio/sound.h>

#include <cstddef>
#include <memory>
#include <span>
#include <vector>

namespace gdk::audio {
    /// \brief transforms one emitter's audio as it is decoded
    ///
    /// This is the library's extension point for audio processing
    ///
    /// An insert effect can be used to implement any kind of effect that depends only on the sound data
    /// itself. E.g: filtering, distortion, delay, gain shaping. 
    /// It does not have any 3D information, so it cannot be used to implement any effects that rely on
    /// spatialisation (e.g: echoing where the echo locations vary from the emitter's location)
    class insert_effect {
    public:
        /// \brief transform aFrames frames in place
        ///
        /// Called from scene::update, once per decoded buffer, on the thread that calls it. The state
        /// an effect keeps is per emitter, since each emitter processes its own audio.
        ///
        /// \param aPCM interleaved samples, aFrames * aChannelCount of them
        /// \param aFrames how many frames are present, which is not always the whole buffer
        /// \param aFormat the layout of aPCM \see sound::format
        /// \param aChannelCount 1 for mono, 2 for stereo
        ///
        /// \warn **must not change the channel count or the frame count.** A stereo source does not
        /// spatialise at all, so widening the signal would silently make the emitter non-spatial.
        virtual void process(std::span<std::byte> aPCM, const pcm_format aFormat,
            const std::size_t aChannelCount) = 0;

        /// \brief return to the state of having processed nothing
        virtual void reset() {}

        virtual ~insert_effect() = default;
    };

    using insert_effect_shared_ptr_type = std::shared_ptr<insert_effect>;
    using insert_effect_collection_type = std::vector<insert_effect_shared_ptr_type>;

    /// \brief a one pole low pass, used for a muffle or occlusion effect
    [[nodiscard]] insert_effect_shared_ptr_type make_lowpass_insert(
        const audio_floating_point_type aCutoffHertz, const std::size_t aSampleRate);

    /// \brief a fixed delay with feedback
    [[nodiscard]] insert_effect_shared_ptr_type make_echo_insert(
        const audio_floating_point_type aDelaySeconds, const audio_floating_point_type aFeedback,
        const audio_floating_point_type aMix, const std::size_t aSampleRate,
        const std::size_t aChannelCount);
}

#endif
