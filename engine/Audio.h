#pragma once

namespace ar {

    class Audio {
    public:
        Audio();
        ~Audio();

        bool Init(int frequency = 44100, int channels = 2, int chunksize = 2048);
        void Shutdown();

        void SetMasterVolume(int percent); // 0-100
        int  GetMasterVolume() const;

        void PauseAll();
        void ResumeAll();
        void StopAll();
    };

} // namespace ar