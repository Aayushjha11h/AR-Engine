#include "Sound.h"
#include <cctype>
#include <algorithm>

namespace ar {

    namespace {
        Sound::Type DetectType(const std::string& path) {
            size_t dot = path.find_last_of('.');
            if (dot == std::string::npos) return Sound::Type::SFX;

            std::string ext = path.substr(dot + 1);
            std::transform(ext.begin(), ext.end(), ext.begin(),
                [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

            if (ext == "ogg" || ext == "mp3" || ext == "flac" || ext == "mod" ||
                ext == "xm" || ext == "s3m" || ext == "it" || ext == "mid")
                return Sound::Type::Music;

            return Sound::Type::SFX;
        }
    }

    Sound::Sound()
        : m_Type(Type::SFX)
        , m_Chunk(nullptr)
        , m_Music(nullptr)
        , m_Volume(100)
        , m_Channel(-1)
    {
    }

    Sound::~Sound() {
        Unload();
    }

    bool Sound::Load(const std::string& path) {
        Unload();
        m_Path = path;
        m_Type = DetectType(path);

        if (m_Type == Type::SFX) {
            m_Chunk = Mix_LoadWAV(path.c_str());
            if (!m_Chunk) return false;
            Mix_VolumeChunk(m_Chunk, (m_Volume * MIX_MAX_VOLUME) / 100);
        }
        else {
            m_Music = Mix_LoadMUS(path.c_str());
            if (!m_Music) return false;
        }
        return true;
    }

    void Sound::Unload() {
        Stop();
        if (m_Chunk) { Mix_FreeChunk(m_Chunk); m_Chunk = nullptr; }
        if (m_Music) { Mix_FreeMusic(m_Music); m_Music = nullptr; }
        m_Path.clear();
        m_Channel = -1;
    }

    void Sound::Play(int loops) {
        if (m_Type == Type::SFX && m_Chunk) {
            m_Channel = Mix_PlayChannel(-1, m_Chunk, loops);
            if (m_Channel != -1)
                Mix_Volume(m_Channel, (m_Volume * MIX_MAX_VOLUME) / 100);
        }
        else if (m_Type == Type::Music && m_Music) {
            Mix_VolumeMusic((m_Volume * MIX_MAX_VOLUME) / 100);
            Mix_PlayMusic(m_Music, loops);
        }
    }

    void Sound::Stop() {
        if (m_Type == Type::SFX) {
            if (m_Channel != -1) {
                Mix_HaltChannel(m_Channel);
                m_Channel = -1;
            }
        }
        else {
            Mix_HaltMusic();
        }
    }

    void Sound::Pause() {
        if (m_Type == Type::SFX) {
            if (m_Channel != -1) Mix_Pause(m_Channel);
        }
        else {
            Mix_PauseMusic();
        }
    }

    void Sound::Resume() {
        if (m_Type == Type::SFX) {
            if (m_Channel != -1) Mix_Resume(m_Channel);
        }
        else {
            Mix_ResumeMusic();
        }
    }

    void Sound::SetVolume(int percent) {
        if (percent < 0) percent = 0;
        if (percent > 100) percent = 100;
        m_Volume = percent;

        if (m_Type == Type::SFX && m_Chunk) {
            Mix_VolumeChunk(m_Chunk, (m_Volume * MIX_MAX_VOLUME) / 100);
            if (m_Channel != -1)
                Mix_Volume(m_Channel, (m_Volume * MIX_MAX_VOLUME) / 100);
        }
    }

    int Sound::GetVolume() const {
        return m_Volume;
    }

    bool Sound::IsPlaying() const {
        if (m_Type == Type::SFX)
            return m_Channel != -1 && Mix_Playing(m_Channel);
        return Mix_PlayingMusic();
    }

    bool Sound::IsPaused() const {
        if (m_Type == Type::SFX)
            return m_Channel != -1 && Mix_Paused(m_Channel);
        return Mix_PausedMusic();
    }

} // namespace ar