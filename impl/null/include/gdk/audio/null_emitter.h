// © Joseph Cameron - All Rights Reserved

#ifndef GDK_AUDIO_NULL_EMITTER_H
#define GDK_AUDIO_NULL_EMITTER_H

#include <gdk/audio/emitter.h>
#include <gdk/audio/types.h>

namespace gdk::audio {
    /// \brief an emitter that makes no sound and reports everything it was asked to do
    class null_emitter final : public emitter {
    public:
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

        [[nodiscard]] virtual bool is_playing() const override;
        virtual void set_looping(const bool aLooping) override;
        [[nodiscard]] virtual bool is_looping() const override;
        virtual void play() override;
        virtual void stop() override;

        void finish();

        [[nodiscard]] bool has_lowpass() const;

        [[nodiscard]] audio_floating_point_type inner_angle() const;
        [[nodiscard]] audio_floating_point_type outer_angle() const;
        [[nodiscard]] audio_floating_point_type outer_gain() const;

        [[nodiscard]] audio_floating_point_type reference_distance() const;
        [[nodiscard]] audio_floating_point_type max_distance() const;
        [[nodiscard]] audio_floating_point_type rolloff_factor() const;

    private:
        vector_type mPosition{0, 0, 0};
        vector_type mVelocity{0, 0, 0};
        vector_type mDirection{0, 0, 0};

        audio_floating_point_type mPitch{1};
        audio_floating_point_type mGain{1};

        audio_floating_point_type mInnerAngle{0};
        audio_floating_point_type mOuterAngle{0};
        audio_floating_point_type mOuterGain{0};

        audio_floating_point_type mReferenceDistance{1};
        audio_floating_point_type mMaxDistance{0};
        audio_floating_point_type mRolloffFactor{1};

        bool mHasLowpass{false};
        bool mPlaying{false};
        bool mLooping{false};
    };
}

#endif
