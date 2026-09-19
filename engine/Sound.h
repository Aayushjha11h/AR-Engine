#pragma once
#include <string>
#include <SDL2/SDL_mixer.h>

namespace ar {

    class Sound {
    public:
        enum class Type { SFX, Music };

        Sound();
        ~Sound();

        bool Load(const std::string& path); // .wav = SFX, .ogg/.mp3/etc = Music
        void Unload();

        void Play(int loops = 0); // 0 = once, -1 = infinite
        void Stop();
        void Pause();
        void Resume();

        void SetVolume(int percent); // 0-100
        int  GetVolume() const;

        bool IsPlaying() const;
        bool IsPaused() const;

        Type GetType() const { return m_Type; }
        const std::string& GetPath() const { return m_Path; }

    private:
        Type m_Type;
        std::string m_Path;
        Mix_Chunk* m_Chunk;
        Mix_Music* m_Music;
        int m_Volume;
        int m_Channel; // last channel used for SFX, -1 if none
    };

} // namespace ar