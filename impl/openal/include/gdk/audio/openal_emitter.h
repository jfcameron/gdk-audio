// © Joseph Cameron - All Rights Reserved

#ifndef GDK_AUDIO_OPENAL_EMITTER_H
#define GDK_AUDIO_OPENAL_EMITTER_H

#include <gdk/audio/emitter.h>
#include <gdk/audio/listener.h>
#include <gdk/audio/sound.h>

#include <jfc/shared_handle.h>

#include <AL/al.h>
#include <AL/alc.h>

#include <memory>
#include <optional>
#include <utility>
#include <vector>

namespace gdk::audio
{
    /// \brief the AL buffer format for a channel count and sample layout
    /// \warning throws if OpenAL has no format for that combination: more than two channels need
    /// AL_EXT_MCFORMATS, and float samples AL_EXT_float32, which the context's OpenAL may lack
    [[nodiscard]] ALenum al_format_from(const std::size_t aChannelCount, const pcm_format aFormat);

    /// \brief root emitter type for openal impl.
    ///
    /// One AL source, or one a channel for an emitter made with a place for each of its sound's
    /// channels: every property set on the emitter reaches all of them, and they start, stop and
    /// pause together, in one call.
    /// \warning in order for 3d spacial effects to be applied to a source, its sound MUST be mono.
    /// A source playing a stereo or multichannel sound plays it flat, as a speaker bed.
    class openal_emitter : public emitter {
        vector_type m_Position = vector_type::zero;
        vector_type m_Velocity = vector_type::zero;

        vector_type m_Direction = vector_type::zero;

        audio_floating_point_type m_Gain = 1;

        //! where each source stands about the emitter's position; zero for the only source
        std::vector<vector_type> m_ChannelOffsets;

        jfc::shared_handle<ALuint> m_LowpassFilter;

        ALuint m_SendSlot = 0;

        std::vector<jfc::shared_handle<ALuint>> m_alSourceHandles;

        bool m_Grouped = false;

    protected:
        enum class state {
            playing,
            paused,
            stopped
        }
        m_state = state::stopped;

        bool m_Looping = false;

        /// \brief every source at once: alSourcePlayv, so they begin in one mix period
        void play_sources();

        void pause_sources();

        void stop_sources();

        /// \brief the first source's offset into what it has queued, in seconds, and how long a
        /// sample mixed now takes to reach the speakers: AL_SOFT_source_latency's, or the offset
        /// alone with no latency where the OpenAL lacks it
        [[nodiscard]] std::pair<double, double> source_offset_and_latency() const;

        /// \brief whether a group restarts this emitter when it runs dry, rather than update()
        [[nodiscard]] bool is_grouped() const;

        /// \brief brings what is queued up to date as a pause comes
        virtual void before_pause() {}

    public:
        /// \brief the underlying OpenAL source: the first, where there are several
        [[nodiscard]] ALuint source_handle() const;

        /// \brief every underlying source: one, or one a channel
        [[nodiscard]] std::vector<ALuint> source_handles() const;

        virtual void set_position(const vector_type &aPosition) override;

        [[nodiscard]] virtual vector_type position() const override;

        virtual void set_velocity(const vector_type &aVelocity) override;

        [[nodiscard]] virtual vector_type velocity() const override;

        virtual void set_pitch(const audio_floating_point_type aPitch) override;

        [[nodiscard]] virtual audio_floating_point_type pitch() const override;

        virtual void set_gain(const audio_floating_point_type aGain) override;

        [[nodiscard]] virtual audio_floating_point_type gain() const override;

        virtual void set_direction(const vector_type &aDirection) override;

        [[nodiscard]] virtual vector_type direction() const override;

        virtual void set_cone(const audio_floating_point_type aInnerAngle,
            const audio_floating_point_type aOuterAngle,
            const audio_floating_point_type aOuterGain) override;

        virtual void set_attenuation(const audio_floating_point_type aReferenceDistance,
            const audio_floating_point_type aMaxDistance,
            const audio_floating_point_type aRolloffFactor) override;

        /// \warning throws where the OpenAL has no effects extension
        virtual void set_lowpass(const lowpass_parameters &aParameters) override;

        virtual void clear_lowpass() override;

        /// \brief route this emitter's signal to a scene's shared effect slot, or to none
        void route_to_send(const ALuint aEffectSlot);

        /// \brief this emitter's lowpass filter object, or AL_FILTER_NULL if it has none
        [[nodiscard]] ALuint lowpass_filter_handle() const;

        /// \brief the effect slot this emitter sends to, or AL_EFFECTSLOT_NULL
        [[nodiscard]] ALuint send_slot() const;

        /// \param aChannelOffsets where each source stands about the emitter: one entry for one
        /// source, or one a channel of a sound played from several places
        explicit openal_emitter(std::vector<vector_type> aChannelOffsets = {vector_type::zero});

        bool is_playing() const override;

        virtual void pause() override;

        virtual void resume() override;

        [[nodiscard]] virtual bool is_paused() const override;

        virtual void set_looping(const bool aLooping) override;

        [[nodiscard]] virtual bool is_looping() const override;

        /// \brief place this emitter's sources relative to the listener that hears them
        void apply_listener(const listener &aListener);

        /// \brief readies the sound to play without starting the sources: what play() does up to
        /// the start, so a group can start several at once. The state becomes playing.
        virtual void begin_play() = 0;

        /// \brief a group restarts a grouped emitter that ran dry, together with the others, so
        /// update() leaves it be
        void set_grouped(const bool aGrouped);

        /// \brief whether play() has been called and neither the sound's end nor stop() reached
        /// since: paused counts, as does having run dry before a refill
        [[nodiscard]] bool wants_to_play() const;

        /// \brief a group paused this emitter's sources itself, with the others' in one call: only
        /// the state is left to note, and what is queued brought up to date
        void note_paused();

        /// \brief a group started this emitter's sources again itself
        void note_resumed();

        virtual void update();
    };
}

#endif
