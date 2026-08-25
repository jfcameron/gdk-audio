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
        signed_16,  //!< two bytes per sample, little endian, silence at 0
        float_32    //!< four bytes per sample, IEEE float, -1 to 1, silence at 0. 
    };

    /// \brief how many bytes one sample of aFormat occupies
    [[nodiscard]] std::size_t bytes_per_sample(const pcm_format aFormat);

    /// \brief a cursor over one sound's decoded audio
    class pcm_stream {
    public:
        /// \brief decode the next frames into aOut
        [[nodiscard]] virtual std::size_t read(std::span<std::byte> aOut) = 0;

        /// \brief return the cursor to the start
        ///
        /// A stream of something live (a microphone, a network radio, a video's soundtrack) has
        /// nowhere to rewind to, and may do nothing here: an emitter's play() then carries on
        /// from where the stream is, and looping it plays whatever comes next
        virtual void rewind() = 0;

        virtual ~pcm_stream() = default;
    };

    /// \brief a playable piece of audio
    class sound {
    public:
        /// \brief 1 for mono, 2 for stereo, 4, 6, 7 or 8 for quad, 5.1, 6.1 and 7.1, **in WAVE's
        /// channel order**, whatever the implementation's own: front left, front right, centre, LFE,
        /// back left, back right, side left, side right, leaving out what the layout has no speaker
        /// for (quad is front left, front right, back left, back right; 6.1 has a back centre where
        /// 7.1 has back left and right). Only a mono sound is placed in space by a plain emitter;
        /// the rest play as a speaker bed, each channel to the matching output, or placed a channel
        /// at a time by scene::make_emitter's channel positions. The backend may lack the formats
        /// past stereo; the scene says so when an emitter is made.
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

    /// \brief build a sound from audio that is already decoded
    ///
    /// \param aFrameCount frames, not samples and not bytes
    /// \warning throws if the description does not match the data
    [[nodiscard]] sound_shared_ptr_type make_pcm_sound(const void *const aData,
        const std::size_t aFrameCount, const std::size_t aChannelCount,
        const std::size_t aSampleRate, const pcm_format aFormat);

    /// \brief creates a new sound from this one but with only PCM data, no decoder
    /// niche use case is for a short sound that is used frequently, where the
    /// increased memory usage of raw PCM is worth paying for the decreased compexity
    /// of not having to manage a decoder and multiple buffers
    [[nodiscard]] sound_shared_ptr_type make_pcm_sound(const sound &aSound);
}

#endif
