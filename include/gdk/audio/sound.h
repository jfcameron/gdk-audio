// © Joseph Cameron - All Rights Reserved

#ifndef GDK_AUDIO_SOUND_H
#define GDK_AUDIO_SOUND_H

#include <gdk/audio/types.h>

#include <cstddef>
#include <span>
#include <memory>

namespace gdk::audio
{
    /// \brief how one sample of decoded audio is laid out
    enum class pcm_format {
        unsigned_8, //!< one unsigned byte per sample, silence at 128
        signed_16   //!< two bytes per sample, little endian, silence at 0
    };

    /// \brief how many bytes one sample of aFormat occupies
    [[nodiscard]] std::size_t bytes_per_sample(const pcm_format aFormat);

    /// \brief a cursor over one sound's decoded audio
    class pcm_stream {
    public:
        /// \brief decode the next frames into aOut
        [[nodiscard]] virtual std::size_t read(std::span<std::byte> aOut) = 0;

        /// \brief return the cursor to the start
        virtual void rewind() = 0;

        virtual ~pcm_stream() = default;
    };

    /// \brief a playable piece of audio: it describes itself, and opens cursors over its samples
    class sound {
    public:
        /// \brief 1 for mono, 2 for stereo. Only a mono sound can be placed in space.
        [[nodiscard]] virtual std::size_t channel_count() const = 0;

        /// \brief frames per second
        [[nodiscard]] virtual std::size_t sample_rate() const = 0;

        /// \brief the layout every stream this sound opens will produce
        [[nodiscard]] virtual pcm_format format() const = 0;

        /// \brief how long the sound plays, in seconds
        [[nodiscard]] virtual audio_floating_point_type duration() const = 0;

        /// \brief open a cursor over this sound's audio
        [[nodiscard]] virtual std::unique_ptr<pcm_stream> open() const = 0;

        virtual ~sound() = default;
    };

    /// \brief build a sound from ogg vorbis data. The library provides this decoder
    /// as living documentation for how to integrate other decoders, and as a convenience
    /// for users who are using ogg vorbis data.
    /// \warn throws if the data is not ogg vorbis
    [[nodiscard]] sound_shared_ptr_type make_vorbis_sound(const unsigned char *const aData,
        const std::size_t aSize);

    /// \brief build a sound from audio that is already decoded
    ///
    /// \param aFrameCount frames, not samples and not bytes
    /// \warn throws if the description does not match the data
    [[nodiscard]] sound_shared_ptr_type make_pcm_sound(const void *const aData,
        const std::size_t aFrameCount, const std::size_t aChannelCount,
        const std::size_t aSampleRate, const pcm_format aFormat);
}

#endif
