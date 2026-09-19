#include "Audio.h"
#include <SDL2/SDL_mixer.h>

namespace ar {

    Audio::Audio() {}
    Audio::~Audio() { Shutdown(); }

    bool Audio::Init(int frequency, int channels, int chunksize) {
        if (Mix_OpenAudio(frequency, MIX_DEFAULT_FORMAT, channels, chunksize) == -1)
            return false;
        Mix_AllocateChannels(32);
        return true;
    }

    void Audio::Shutdown() {
        Mix_CloseAudio();
    }

    void Audio::SetMasterVolume(int percent) {
        if (percent < 0) percent = 0;
        if (percent > 100) percent = 100;
        int vol = (percent * MIX_MAX_VOLUME) / 100;
        Mix_Volume(-1, vol);      // all SFX channels
        Mix_VolumeMusic(vol);     // music channel
    }

    int Audio::GetMasterVolume() const {
        int vol = Mix_Volume(-1, -1);
        return (vol * 100) / MIX_MAX_VOLUME;
    }

    void Audio::PauseAll() {
        Mix_Pause(-1);
        Mix_PauseMusic();
    }

    void Audio::ResumeAll() {
        Mix_Resume(-1);
        Mix_ResumeMusic();
    }

    void Audio::StopAll() {
        Mix_HaltChannel(-1);
        Mix_HaltMusic();
    }

} // namespace ar