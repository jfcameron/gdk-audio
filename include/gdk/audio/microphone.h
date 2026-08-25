// © Joseph Cameron - All Rights Reserved

#ifndef GDK_AUDIO_MICROPHONE_H
#define GDK_AUDIO_MICROPHONE_H

#include <gdk/audio/sound.h>
#include <gdk/audio/types.h>

#include <cstddef>
#include <span>

namespace gdk::audio {
    /// \brief captures audio from an input device
    ///
    /// Example:
    ///
    /// ~~~{.cpp}
    /// auto pMic = pContext->make_microphone({}); // system default device, default format
    ///
    /// pMic->start();
    ///
    /// // ... once a frame ...
    /// pMic->update();
    ///
    /// pMic->stop();
    ///
    /// auto pSound = pMic->take(); // everything captured, as one sound
    /// ~~~
    ///
    class microphone {
    public:
        /// \brief what a caller asks a device for
        struct request final {
            std::string device_name;

            std::size_t sample_rate{44100};

            std::size_t channel_count{1};

            pcm_format format{pcm_format::signed_16};

            std::size_t capacity_in_frames{44100 * 5};

            bool drop_oldest_when_full{false};
        };

        [[nodiscard]] virtual std::size_t sample_rate() const = 0;

        [[nodiscard]] virtual std::size_t channel_count() const = 0;

        [[nodiscard]] virtual pcm_format format() const = 0;

        [[nodiscard]] virtual std::string_view device_name() const = 0;

        [[nodiscard]] virtual std::size_t capacity_in_frames() const = 0;

        /// \brief begin capturing
        ///
        /// \warn throws if the device cannot be started. 
        virtual void start() = 0;

        //! stop capturing. What was captured is kept until taken.
        virtual void stop() = 0;

        //! whether this is capturing right now
        [[nodiscard]] virtual bool capturing() const = 0;

        /// \brief move whatever the device has captured into this object's buffer
        ///
        /// \attention must be called in your update loop while capturing. The device's own buffer is
        /// small and overruns quietly.
        virtual void update() = 0;

        //! how many frames are held and not yet taken
        [[nodiscard]] virtual std::size_t available_in_frames() const = 0;

        //! whether the buffer has reached \ref capacity_in_frames
        [[nodiscard]] virtual bool capacity_reached() const = 0;

        /// \brief how many frames the cap policy has discarded
        [[nodiscard]] virtual std::size_t dropped_frame_count() const = 0;

        /// \brief take everything captured so far as one sound, emptying the buffer
        [[nodiscard]] virtual sound_shared_ptr_type take() = 0;

        /// \brief copy out up to aFrameCount frames, removing them from the buffer
        virtual std::size_t read(std::span<std::byte> aOut) = 0;

        //! throw away what is held without taking it
        virtual void discard() = 0;

        virtual ~microphone() = default;
    };
}

#endif

