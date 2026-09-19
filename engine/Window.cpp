#include "Window.h"
#include "Input.h"
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <iostream>

namespace ar {

    Window::Window() : m_Window(nullptr), m_GLContext(nullptr), m_Width(0), m_Height(0), m_ShouldClose(false) {}
    Window::~Window() { Shutdown(); }

    bool Window::Init(const std::string& title, int width, int height) {
        m_Width = width; m_Height = height;
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
            std::cerr << "[AR] SDL_Init failed: " << SDL_GetError() << "\n";
            return false;
        }
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

        m_Window = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            width, height, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
        if (!m_Window) { std::cerr << "[AR] Window failed\n"; return false; }

        m_GLContext = SDL_GL_CreateContext(m_Window);
        if (!m_GLContext) { std::cerr << "[AR] GL Context failed\n"; return false; }

        glewExperimental = GL_TRUE;
        if (glewInit() != GLEW_OK) { std::cerr << "[AR] GLEW failed\n"; return false; }

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glViewport(0, 0, width, height);
        std::cout << "[AR] OpenGL " << glGetString(GL_VERSION) << "\n";
        return true;
    }

    void Window::Shutdown() {
        if (m_GLContext) { SDL_GL_DeleteContext(m_GLContext); m_GLContext = nullptr; }
        if (m_Window) { SDL_DestroyWindow(m_Window); m_Window = nullptr; }
        SDL_Quit();
    }

    void Window::PollEvents(Input& input) {
        input.NewFrame();
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            input.ProcessEvent(e);
            if (e.type == SDL_QUIT) m_ShouldClose = true;
            else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) m_ShouldClose = true;
            else if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_RESIZED) {
                m_Width = e.window.data1; m_Height = e.window.data2;
                glViewport(0, 0, m_Width, m_Height);
            }
        }
    }

    void Window::SwapBuffers() { SDL_GL_SwapWindow(m_Window); }

} // namespace ar