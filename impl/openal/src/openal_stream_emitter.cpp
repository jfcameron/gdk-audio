// © Joseph Cameron - All Rights Reserved

#include <gdk/audio/exception.h>
#include <gdk/audio/openal_stream_emitter.h>

#include <algorithm>
#include <cstring>
#include <limits>
#include <span>
#include <string>

namespace gdk::audio
{
    openal_stream_emitter::openal_stream_emitter(const sound_shared_ptr_type &apSound,
        const openal_policy &aPolicy, insert_effect_collection_type aInserts,
        std::vector<vector_type> aChannelOffsets)
    : openal_emitter([&]()
    {
        if (!aChannelOffsets.empty() && aChannelOffsets.size() != apSound->channel_count())
            throw exception("an emitter placing a sound's channels needs a place for each: the sound has "
                + std::to_string(apSound->channel_count()) + " channels, and "
                + std::to_string(aChannelOffsets.size()) + " places were given");

        return aChannelOffsets;
    }())
    , m_Policy(aPolicy)
    , m_Inserts(std::move(aInserts))
    , m_pSound(apSound)
    , m_pStream(apSound->open())
    , m_BytesPerFrame(apSound->channel_count() * bytes_per_sample(apSound->format()))
    , m_BytesPerSample(bytes_per_sample(apSound->format()))
    , m_Format(al_format_from(aChannelOffsets.size() > 1 ? 1 : apSound->channel_count(), apSound->format()))
    , m_SampleRate(static_cast<ALsizei>(apSound->sample_rate()))
    , m_alBufferHandles([&]()
    {
        const auto count = std::max<std::size_t>(2, aPolicy.STREAM_BUFFER_COUNT);
        const auto sources = std::max<std::size_t>(1, aChannelOffsets.size());

        std::vector<std::vector<jfc::shared_handle<ALuint>>> handles(sources);

        for (auto &ofSource : handles)
        {
            ofSource.reserve(count);

            for (std::size_t i = 0; i < count; ++i) ofSource.emplace_back([]()
            {
                ALuint handle;

                alGenBuffers(1, &handle);

                return handle;
            }(),
            [](const ALuint a) { alDeleteBuffers(1, &a); });
        }

        return handles;
    }())
    {
        if (!m_pStream) throw exception("a sound handed back no stream to decode");

        m_PCMBuffer.resize(m_Policy.STREAM_FRAMES_PER_BUFFER * m_BytesPerFrame);

        if (is_split()) m_ChannelBuffer.resize(m_Policy.STREAM_FRAMES_PER_BUFFER * m_BytesPerSample);

        m_FramesInSlot.assign(m_alBufferHandles.front().size(), 0);

        for (const auto &ofSource : m_alBufferHandles)
            for (std::size_t slot = 0; slot < ofSource.size(); ++slot) m_SlotOfBuffer[ofSource[slot].get()] = slot;
    }

    bool openal_stream_emitter::is_split() const
    {
        return m_alBufferHandles.size() > 1;
    }

    void openal_stream_emitter::begin_play()
    {
        m_pStream->rewind();

        for (const auto &pInsert : m_Inserts) pInsert->reset();

        unqueue_all();

        m_FramesRetired = 0;

        queue_from_stream();

        m_state = state::playing;
    }

    void openal_stream_emitter::play()
    {
        if (m_state != state::stopped) return;

        begin_play();

        play_sources();
    }

    void openal_stream_emitter::queue_from_stream()
    {
        for (std::size_t slot = 0; slot < m_FramesInSlot.size(); ++slot)
        {
            if (!decode_next_samples(slot)) break;

            for (std::size_t source = 0; source < m_alBufferHandles.size(); ++source)
            {
                const auto buffer = m_alBufferHandles[source][slot].get();

                alSourceQueueBuffers(source_handles()[source], 1, &buffer);
            }
        }
    }

    void openal_stream_emitter::unqueue_all()
    {
        for (const auto handle : source_handles())
        {
            ALint queued = 0;
            alGetSourcei(handle, AL_BUFFERS_QUEUED, &queued);

            for (ALint i = 0; i < queued; ++i)
            {
                ALuint which;
                alSourceUnqueueBuffers(handle, 1, &which);
            }
        }
    }

    void openal_stream_emitter::refill()
    {
        const auto handles = source_handles();

        ALint processed = std::numeric_limits<ALint>::max();

        for (const auto handle : handles)
        {
            ALint ofSource = 0;
            alGetSourcei(handle, AL_BUFFERS_PROCESSED, &ofSource);

            processed = std::min(processed, ofSource);
        }

        while (processed-- > 0)
        {
            std::optional<std::size_t> slot;

            for (std::size_t source = 0; source < handles.size(); ++source)
            {
                ALuint which;
                alSourceUnqueueBuffers(handles[source], 1, &which);

                if (!source)
                    if (const auto found = m_SlotOfBuffer.find(which); found != m_SlotOfBuffer.end())
                    {
                        slot = found->second;

                        m_FramesRetired += m_FramesInSlot[*slot];
                    }
            }

            if (!slot || m_state != state::playing || !decode_next_samples(*slot)) continue;

            for (std::size_t source = 0; source < handles.size(); ++source)
            {
                const auto buffer = m_alBufferHandles[source][*slot].get();

                alSourceQueueBuffers(handles[source], 1, &buffer);
            }
        }
    }

    void openal_stream_emitter::before_pause() {
        refill();
    }

    void openal_stream_emitter::update() {
        if (m_state == state::paused) return;

        refill();

        ALint queuedBufferCount;
        alGetSourcei(source_handle(), AL_BUFFERS_QUEUED, &queuedBufferCount);

        if (!queuedBufferCount)
        {
            m_state = state::stopped;
        }
        else if (m_state == state::playing && !is_grouped())
        {
            ALint state;
            alGetSourcei(source_handle(), AL_SOURCE_STATE, &state);

            if (state != AL_PLAYING) play_sources();
        }
    }

    bool openal_stream_emitter::decode_next_samples(const std::size_t aSlot)
    {
        auto frames = m_pStream->read(std::as_writable_bytes(std::span(m_PCMBuffer)));

        if (!frames && m_Looping)
        {
            m_pStream->rewind();

            frames = m_pStream->read(std::as_writable_bytes(std::span(m_PCMBuffer)));
        }

        if (!frames) return false;

        for (const auto &pInsert : m_Inserts)
            pInsert->process(std::as_writable_bytes(std::span(m_PCMBuffer))
                    .first(frames * m_BytesPerFrame), m_pSound->format(),
                m_pSound->channel_count());

        m_FramesInSlot[aSlot] = frames;

        if (!is_split())
        {
            alBufferData(m_alBufferHandles.front()[aSlot].get(), m_Format, m_PCMBuffer.data(),
                static_cast<ALsizei>(frames * m_BytesPerFrame), m_SampleRate);

            return true;
        }

        const auto channels = m_alBufferHandles.size();

        for (std::size_t channel = 0; channel < channels; ++channel)
        {
            for (std::size_t frame = 0; frame < frames; ++frame)
                std::memcpy(m_ChannelBuffer.data() + frame * m_BytesPerSample,
                    m_PCMBuffer.data() + (frame * channels + channel) * m_BytesPerSample, m_BytesPerSample);

            alBufferData(m_alBufferHandles[channel][aSlot].get(), m_Format, m_ChannelBuffer.data(),
                static_cast<ALsizei>(frames * m_BytesPerSample), m_SampleRate);
        }

        return true;
    }

    void openal_stream_emitter::stop()
    {
        stop_sources();

        unqueue_all();

        m_FramesRetired = 0;

        m_state = state::stopped;
    }

    void openal_stream_emitter::discard_queued()
    {
        if (m_state == state::stopped) return;

        const auto [offset, latency] = source_offset_and_latency();

        stop_sources();

        unqueue_all();

        m_FramesRetired += static_cast<std::size_t>(std::max(0.0, offset * m_SampleRate));

        queue_from_stream();

        if (m_state == state::playing) play_sources();
    }

    std::optional<double> openal_stream_emitter::playback_position() const
    {
        if (m_state == state::stopped) return {};

        const auto [offset, latency] = source_offset_and_latency();

        return std::max(0.0, static_cast<double>(m_FramesRetired) + (offset - latency) * m_SampleRate);
    }
}
