// © 2019 Joseph Cameron - All Rights Reserved

#ifndef GDK_AUDIO_OPENAL_EMITTER_H
#define GDK_AUDIO_OPENAL_EMITTER_H

#include <gdk/audio/emitter.h>
#include <gdk/audio/openal_sound.h>

#include <jfc/shared_handle.h>

#include <AL/al.h>
#include <AL/alc.h>

#include <memory>

namespace gdk::audio
{
    /// \brief root emitter type for openal impl.
    /// provides a common ancestor for collections etc that do not care about
    /// the specific differences between streamers (multi buffer, constant decoding) and 
	// simples (single buffer, 1 decode at sound ctor time)
	/// \warn in order for 3d spacial effects to be applied to the emitter, the sound provided to the emitter
	/// MUST be mono. Stereo sounds will always be played without these effects.
    class openal_emitter : public emitter
    {
		vector_type m_LastPosition;
		vector_type m_Direction;
		vector_type m_Speed;

	//protected: //TODO REMOVE
		//! 3d position of the emitter in the sound space
        jfc::shared_handle<ALuint> m_alSourceHandle;

    protected:
        enum class state
        {
            playing,
            stopped
        }
        m_state = state::stopped;

        ALuint getSourceHandle();

    public:
		virtual void set_position(const vector_type& aPosition) override;

		virtual void set_pitch(const float aPitch) override;

        openal_emitter();

        bool isPlaying() const override;

        virtual void update();
    };
}

#endif
