// © Joseph Cameron - MIT License

#ifndef GDK_AUDIO_TYPES_H
#define GDK_AUDIO_TYPES_H

#include <gdk/math/ops.h>
#include <gdk/math/quaternion.h>
#include <gdk/math/vector3.h>

#include <memory>

namespace gdk::audio {
    class context;
    class emitter;
    class emitter_group;
    class listener;
    class microphone;
    class scene;
    class sound;

    using audio_floating_point_type = float;

    using audio_vector3_type = gdk::math::vector3<audio_floating_point_type>;
    using audio_quaternion_type = gdk::math::quaternion<audio_floating_point_type>;

    using context_unique_ptr_type = std::unique_ptr<context>;
    using context_shared_ptr_type = std::shared_ptr<context>;

    using emitter_shared_ptr_type = std::shared_ptr<emitter>;
    using emitter_group_shared_ptr_type = std::shared_ptr<emitter_group>;
    using microphone_shared_ptr_type = std::shared_ptr<microphone>;
    using scene_shared_ptr_type = std::shared_ptr<scene>;
    using sound_shared_ptr_type = std::shared_ptr<sound>;
}

#endif
