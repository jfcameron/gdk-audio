// © Joseph Cameron - All Rights Reserved

#ifndef GDK_AUDIO_OPENAL_EMITTER_H
#define GDK_AUDIO_OPENAL_EMITTER_H

#include <gdk/audio/emitter.h>
#include <gdk/audio/listener.h>
#include <gdk/audio/sound.h>

#include <jfc/shared_handle.h>

#include <AL/al.h>
#include <AL/alc.h>

#define AL_ALEXT_PROTOTYPES
#include <AL/efx.h>

#include <memory>

namespace gdk::audio
{
    /// \brief the AL buffer format for a channel count and sample layout
    /// \warn throws if OpenAL has no format for that combination
    [[nodiscard]] ALenum al_format_from(const std::size_t aChannelCount, const pcm_format aFormat);

    /// \brief root emitter type for openal impl.
	/// \warn in order for 3d spacial effects to be applied to the emitter, the sound provided to the emitter
	/// MUST be mono. Stereo sounds will always be played without these effects.
    class openal_emitter : public emitter {
        vector_type m_Position = vector_type::zero;
        vector_type m_Velocity = vector_type::zero;

        //! zero means omnidirectional, which is what OpenAL takes a zero AL_DIRECTION to mean too
        vector_type m_Direction = vector_type::zero;

        audio_floating_point_type m_Gain = 1;

        std::unique_ptr<jfc::shared_handle<ALuint>> m_pLowpassFilter;

        ALuint m_SendSlot = AL_EFFECTSLOT_NULL;

        jfc::shared_handle<ALuint> m_alSourceHandle;

    protected:
        enum class state {
            playing,
            stopped
        }
        m_state = state::stopped;

        bool m_Looping = false;

    public:
        /// \brief the underlying OpenAL source
        [[nodiscard]] ALuint source_handle() const;

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

        virtual void set_lowpass(const lowpass_parameters &aParameters) override;

        virtual void clear_lowpass() override;

        /// \brief route this emitter's signal to a scene's shared effect slot, or to none
        void route_to_send(const ALuint aEffectSlot);

        /// \brief this emitter's lowpass filter object, or AL_FILTER_NULL if it has none
        [[nodiscard]] ALuint lowpass_filter_handle() const;

        /// \brief the effect slot this emitter sends to, or AL_EFFECTSLOT_NULL
        [[nodiscard]] ALuint send_slot() const;

        openal_emitter();

        bool is_playing() const override;

        virtual void set_looping(const bool aLooping) override;

        [[nodiscard]] virtual bool is_looping() const override;

        /// \brief place this emitter relative to the listener that hears it
        void apply_listener(const listener &aListener);

        virtual void update();
    };
}

#endif
