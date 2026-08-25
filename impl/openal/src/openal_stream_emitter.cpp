// © Joseph Cameron - All Rights Reserved

#include <gdk/audio/exception.h>
#include <gdk/audio/openal_stream_emitter.h>

#include <algorithm>
#include <span>

namespace gdk::audio
{
    openal_stream_emitter::openal_stream_emitter(const sound_shared_ptr_type &apSound,
        const openal_policy &aPolicy, insert_effect_collection_type aInserts)
    : openal_emitter()
    , m_Policy(aPolicy)
    , m_Inserts(std::move(aInserts))
    , m_pSound(apSound)
    , m_pStream(apSound->open())
    , m_BytesPerFrame(apSound->channel_count() * bytes_per_sample(apSound->format()))
    , m_Format(al_format_from(apSound->channel_count(), apSound->format()))
    , m_SampleRate(static_cast<ALsizei>(apSound->sample_rate()))
    , m_alBufferHandles([&aPolicy]()
    {
        const auto count = std::max<std::size_t>(2, aPolicy.STREAM_BUFFER_COUNT);

        std::vector<jfc::shared_handle<ALuint>> handles;
        handles.reserve(count);

        for (std::size_t i = 0; i < count; ++i) handles.emplace_back([]()
        {
            ALuint handle;

            alGenBuffers(1, &handle);

            return handle;
        }(),
        [](const ALuint a) { alDeleteBuffers(1, &a); });

        return handles;
    }())
    {
        if (!m_pStream) throw exception("a sound handed back no stream to decode");

        m_PCMBuffer.resize(m_Policy.STREAM_FRAMES_PER_BUFFER * m_BytesPerFrame);
    }

    void openal_stream_emitter::play()
    {
        if (m_state != state::stopped) return;

        m_pStream->rewind();

        for (const auto &pInsert : m_Inserts) pInsert->reset();

        for (auto &handle : m_alBufferHandles)
        {
            const auto buffer = handle.get();

            if (decode_next_samples(buffer)) alSourceQueueBuffers(source_handle(), 1, &buffer);
        }

        alSourcePlay(source_handle());

        m_state = state::playing;
    }

    void openal_stream_emitter::update()
    {
        const auto sourceHandle = source_handle();

        ALint processed;
        alGetSourcei(sourceHandle, AL_BUFFERS_PROCESSED, &processed);

        if (processed)
        {
            ALuint which;
            alSourceUnqueueBuffers(sourceHandle, 1, &which);

            if (m_state == state::playing && decode_next_samples(which))
                alSourceQueueBuffers(sourceHandle, 1, &which);
        }

        ALint queuedBufferCount;
        alGetSourcei(sourceHandle, AL_BUFFERS_QUEUED, &queuedBufferCount);

        if (!queuedBufferCount)
        {
            m_state = state::stopped;
        }
        else if (m_state == state::playing)
        {
            ALint state;
            alGetSourcei(sourceHandle, AL_SOURCE_STATE, &state);

            if (state != AL_PLAYING) alSourcePlay(sourceHandle);
        }
    }

    bool openal_stream_emitter::decode_next_samples(ALuint aOutputPCMBuffer)
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

        alBufferData(aOutputPCMBuffer, m_Format, m_PCMBuffer.data(),
            static_cast<ALsizei>(frames * m_BytesPerFrame), m_SampleRate);

        return true;
    }

    void openal_stream_emitter::stop()
    {
        alSourceStop(source_handle());

        ALint queued;
        alGetSourcei(source_handle(), AL_BUFFERS_QUEUED, &queued);

        for (ALint i = 0; i < queued; ++i)
        {
            ALuint which;
            alSourceUnqueueBuffers(source_handle(), 1, &which);
        }

        m_state = state::stopped;
    }
}
