#pragma once
#include <string>

struct SDL_Window;

namespace ar {
    class Input;

    class Window {
    public:
        Window();
        ~Window();
        bool Init(const std::string& title, int width, int height);
        void Shutdown();
        void PollEvents(Input& input);
        void SwapBuffers();
        bool ShouldClose() const { return m_ShouldClose; }
        void SetShouldClose(bool c) { m_ShouldClose = c; }
        int GetWidth() const { return m_Width; }
        int GetHeight() const { return m_Height; }
        SDL_Window* GetNativeWindow() const { return m_Window; }

    private:
        SDL_Window* m_Window;
        void* m_GLContext;
        int m_Width, m_Height;
        bool m_ShouldClose;
    };

} // namespace ar