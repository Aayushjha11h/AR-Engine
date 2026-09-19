#pragma once
#include <SDL2/SDL.h>
#include <glm/glm.hpp>

namespace ar {

    class Input {
    public:
        static constexpr int KEY_COUNT = 512;
        static constexpr int MOUSE_BUTTON_COUNT = 5;

        void ProcessEvent(const SDL_Event& e);
        void NewFrame();

        bool IsKeyDown(int scancode) const;
        bool IsKeyPressed(int scancode) const;
        bool IsKeyReleased(int scancode) const;

        bool IsMouseDown(int button) const;
        bool IsMousePressed(int button) const;
        bool IsMouseReleased(int button) const;

        glm::vec2 GetMousePos() const { return m_MousePos; }
        glm::vec2 GetMouseDelta() const { return m_MousePos - m_LastMousePos; }
        float GetMouseScroll() const { return m_MouseScroll; }

    private:
        bool m_Keys[KEY_COUNT] = { false };
        bool m_PrevKeys[KEY_COUNT] = { false };
        bool m_Mouse[MOUSE_BUTTON_COUNT] = { false };
        bool m_PrevMouse[MOUSE_BUTTON_COUNT] = { false };
        glm::vec2 m_MousePos = { 0,0 };
        glm::vec2 m_LastMousePos = { 0,0 };
        float m_MouseScroll = 0.0f;
    };

} // namespace ar