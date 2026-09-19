#include "Timer.h"
#include <SDL2/SDL.h>

namespace ar {

    Timer::Timer()
        : m_Now(0), m_Last(0), m_Start(0), m_DeltaTime(0.0f), m_TargetFrameTicks(0)
    {
    }

    void Timer::Start() {
        m_Start = SDL_GetPerformanceCounter();
        m_Last = m_Start;
        m_Now = m_Start;
        m_DeltaTime = 0.0f;
    }

    void Timer::Tick() {
        m_Last = m_Now;
        m_Now = SDL_GetPerformanceCounter();

        uint64_t freq = SDL_GetPerformanceFrequency();
        m_DeltaTime = (m_Now - m_Last) / static_cast<float>(freq);

        // Prevent spiral of death if debugging
        if (m_DeltaTime > 0.25f) m_DeltaTime = 0.25f;
    }

    float Timer::GetElapsedTime() const {
        uint64_t freq = SDL_GetPerformanceFrequency();
        return (SDL_GetPerformanceCounter() - m_Start) / static_cast<float>(freq);
    }

    void Timer::SetTargetFPS(uint32_t fps) {
        if (fps == 0) {
            m_TargetFrameTicks = 0;
            return;
        }
        uint64_t freq = SDL_GetPerformanceFrequency();
        m_TargetFrameTicks = freq / fps;
    }

    void Timer::DelayIfNeeded() {
        if (m_TargetFrameTicks == 0) return;

        uint64_t frameTicks = m_Now - m_Last;
        if (frameTicks < m_TargetFrameTicks) {
            uint64_t delayMs = (m_TargetFrameTicks - frameTicks) * 1000 / SDL_GetPerformanceFrequency();
            if (delayMs > 0) SDL_Delay(static_cast<uint32_t>(delayMs));
        }
    }

} // namespace ar