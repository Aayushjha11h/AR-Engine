#pragma once
#include <cstdint>

namespace ar {

    class Timer {
    public:
        Timer();

        void Start();
        void Tick();

        float GetDeltaTime() const { return m_DeltaTime; }
        float GetElapsedTime() const;

        // Cap FPS. 0 = uncapped.
        void SetTargetFPS(uint32_t fps);
        void DelayIfNeeded();

    private:
        uint64_t m_Now;
        uint64_t m_Last;
        uint64_t m_Start;
        float m_DeltaTime;
        uint64_t m_TargetFrameTicks;
    };

} // namespace ar