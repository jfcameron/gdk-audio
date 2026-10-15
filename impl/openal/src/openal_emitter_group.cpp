// © Joseph Cameron - All Rights Reserved

#include <gdk/audio/openal_emitter_group.h>

namespace gdk::audio
{
    openal_emitter_group::openal_emitter_group(std::vector<std::shared_ptr<openal_emitter>> aMembers)
    {
        m_Members.reserve(aMembers.size());

        for (const auto &pMember : aMembers)
        {
            pMember->set_grouped(true);

            m_Members.push_back(pMember);
        }
    }

    openal_emitter_group::~openal_emitter_group()
    {
        for (const auto &pMember : members()) pMember->set_grouped(false);
    }

    std::vector<std::shared_ptr<openal_emitter>> openal_emitter_group::members() const
    {
        std::vector<std::shared_ptr<openal_emitter>> out;

        for (const auto &member : m_Members) if (auto pMember = member.lock()) out.push_back(std::move(pMember));

        return out;
    }

    std::vector<ALuint> openal_emitter_group::sources() const
    {
        std::vector<ALuint> out;

        for (const auto &pMember : members())
            for (const auto handle : pMember->source_handles()) out.push_back(handle);

        return out;
    }

    void openal_emitter_group::play()
    {
        const auto living = members();

        for (const auto &pMember : living) pMember->stop();

        for (const auto &pMember : living) pMember->begin_play();

        const auto handles = sources();

        alSourcePlayv(static_cast<ALsizei>(handles.size()), handles.data());
    }

    void openal_emitter_group::pause()
    {
        const auto handles = sources();

        alSourcePausev(static_cast<ALsizei>(handles.size()), handles.data());

        for (const auto &pMember : members()) pMember->note_paused();
    }

    void openal_emitter_group::resume()
    {
        std::vector<ALuint> paused;
        std::vector<std::shared_ptr<openal_emitter>> resumed;

        for (const auto &pMember : members())
        {
            if (!pMember->is_paused()) continue;

            for (const auto handle : pMember->source_handles()) paused.push_back(handle);

            resumed.push_back(pMember);
        }

        if (paused.empty()) return;

        alSourcePlayv(static_cast<ALsizei>(paused.size()), paused.data());

        for (const auto &pMember : resumed) pMember->note_resumed();
    }

    void openal_emitter_group::stop()
    {
        const auto handles = sources();

        alSourceStopv(static_cast<ALsizei>(handles.size()), handles.data());

        for (const auto &pMember : members()) pMember->stop();
    }

    bool openal_emitter_group::is_playing() const
    {
        for (const auto &pMember : members()) if (pMember->is_playing()) return true;

        return false;
    }

    bool openal_emitter_group::is_paused() const
    {
        const auto living = members();

        if (living.empty()) return false;

        for (const auto &pMember : living) if (!pMember->is_paused()) return false;

        return true;
    }

    std::size_t openal_emitter_group::size() const
    {
        return members().size();
    }

    void openal_emitter_group::update()
    {
        std::vector<ALuint> playing;
        bool dry = false;

        for (const auto &pMember : members())
        {
            if (!pMember->wants_to_play() || pMember->is_paused()) continue;

            for (const auto handle : pMember->source_handles())
            {
                ALint queued = 0;
                alGetSourcei(handle, AL_BUFFERS_QUEUED, &queued);

                ALint state = AL_STOPPED;
                alGetSourcei(handle, AL_SOURCE_STATE, &state);

                if (queued && state != AL_PLAYING) dry = true;

                if (queued) playing.push_back(handle);
            }
        }

        if (!dry || playing.empty()) return;

        alSourcePausev(static_cast<ALsizei>(playing.size()), playing.data());
        alSourcePlayv(static_cast<ALsizei>(playing.size()), playing.data());
    }
}
