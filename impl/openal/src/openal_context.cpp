// © Joseph Cameron - All Rights Reserved

#include <gdk/audio/openal_context.h>

#include <gdk/audio/openal_microphone.h>
#include <gdk/audio/exception.h>
#include <gdk/audio/openal_stream_emitter.h>
#include <gdk/audio/openal_simple_emitter.h>

#include <iostream>
#include <fstream>
#include <string>

static inline std::string al_error_code_to_string(ALenum aError) {
	switch (aError) {
		case AL_NO_ERROR: return "AL_NO_ERROR";
		case AL_INVALID_NAME: return "AL_INVALID_NAME";
		case AL_INVALID_ENUM: return "AL_INVALID_ENUM";
		case AL_INVALID_VALUE: return "AL_INVALID_VALUE";
		case AL_INVALID_OPERATION: return "AL_INVALID_OPERATION";
		case AL_OUT_OF_MEMORY: return "AL_OUT_OF_MEMORY";
	}

	return "unrecognized OpenAL error code " + std::to_string(aError);
}

namespace gdk::audio {
    namespace {
        [[nodiscard]] ALenum al_distance_model_from(const distance_model aModel) {
            switch (aModel) {
                case distance_model::none: return AL_NONE;
                case distance_model::inverse: return AL_INVERSE_DISTANCE;
                case distance_model::inverse_clamped: return AL_INVERSE_DISTANCE_CLAMPED;
                case distance_model::linear: return AL_LINEAR_DISTANCE;
                case distance_model::linear_clamped: return AL_LINEAR_DISTANCE_CLAMPED;
                case distance_model::exponent: return AL_EXPONENT_DISTANCE;
                case distance_model::exponent_clamped: return AL_EXPONENT_DISTANCE_CLAMPED;
            }

            throw exception("unrecognized distance model");
        }

        bool aContextIsLive = false;
    }

    context_unique_ptr_type openal_context::make(openal_policy aPolicy)
    {
        return context_unique_ptr_type(new openal_context(std::move(aPolicy)));
    }

    openal_context::~openal_context()
    {
        aContextIsLive = false;
    }

    openal_context::openal_context(openal_policy aPolicy)
    : m_Policy(std::move(aPolicy))
    , m_pCurrentDevice({
        []()
        {
			if (aContextIsLive) throw exception("a gdk::audio context is already live. OpenAL "
				"makes one context current per process, so a second would silence the first");

			auto device_buffer = alcOpenDevice(0); 

			if (!device_buffer) 
				throw exception("could not initialize context on audio device");

			return device_buffer;
        }(),
        [](ALCdevice *const p)
        {
            alcCloseDevice(p);
        }})
    , m_pContext({
        [this]()
        {
            auto context_buffer = alcCreateContext(m_pCurrentDevice.get(), 0);

			alcProcessContext(context_buffer);

            if (!alcMakeContextCurrent(context_buffer)) 
				throw exception("could not make initial audio context current");

            return context_buffer;
        }(), 
        [](ALCcontext *const p)
        {
            alcDestroyContext(p);
        }})
    {
        if (const auto error = alGetError(); error != AL_NO_ERROR) 
            throw exception(al_error_code_to_string(error));

        aContextIsLive = true;

		alDistanceModel(al_distance_model_from(m_Policy.DISTANCE_MODEL));
		alDopplerFactor(m_Policy.DOPPLER_FACTOR);
		alSpeedOfSound(m_Policy.SPEED_OF_SOUND);
    }
    
    std::vector<std::string> openal_context::capture_device_names() const {
	if (!alcIsExtensionPresent(nullptr, "ALC_EXT_CAPTURE")) return {};

	const auto *pAt = alcGetString(nullptr, ALC_CAPTURE_DEVICE_SPECIFIER);

	if (!pAt) return {};

	std::vector<std::string> out;

	while (*pAt) {
		out.emplace_back(pAt);

		pAt += out.back().size() + 1;
	}

	return out;
}

microphone_shared_ptr_type openal_context::make_microphone(const microphone::request &aRequest) {
	return std::make_shared<openal_microphone>(aRequest);
}

scene_shared_ptr_type openal_context::make_scene()
    {
        return openal_scene::make(m_Policy);
    }

}

