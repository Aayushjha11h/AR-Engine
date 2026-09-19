#include "Input.h"

namespace ar {

    void Input::ProcessEvent(const SDL_Event& e) {
        if (e.type == SDL_KEYDOWN && e.key.keysym.scancode < KEY_COUNT)
            m_Keys[e.key.keysym.scancode] = true;
        else if (e.type == SDL_KEYUP && e.key.keysym.scancode < KEY_COUNT)
            m_Keys[e.key.keysym.scancode] = false;
        else if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button < MOUSE_BUTTON_COUNT)
            m_Mouse[e.button.button] = true;
        else if (e.type == SDL_MOUSEBUTTONUP && e.button.button < MOUSE_BUTTON_COUNT)
            m_Mouse[e.button.button] = false;
        else if (e.type == SDL_MOUSEMOTION)
            m_MousePos = { (float)e.motion.x, (float)e.motion.y };
        else if (e.type == SDL_MOUSEWHEEL)
            m_MouseScroll += e.wheel.preciseY;
    }

    void Input::NewFrame() {
        for (int i = 0; i < KEY_COUNT; ++i) m_PrevKeys[i] = m_Keys[i];
        for (int i = 0; i < MOUSE_BUTTON_COUNT; ++i) m_PrevMouse[i] = m_Mouse[i];
        m_LastMousePos = m_MousePos;
        m_MouseScroll = 0.0f;
    }

    bool Input::IsKeyDown(int s) const { return s >= 0 && s < KEY_COUNT ? m_Keys[s] : false; }
    bool Input::IsKeyPressed(int s) const { return s >= 0 && s < KEY_COUNT ? (m_Keys[s] && !m_PrevKeys[s]) : false; }
    bool Input::IsKeyReleased(int s) const { return s >= 0 && s < KEY_COUNT ? (!m_Keys[s] && m_PrevKeys[s]) : false; }

    bool Input::IsMouseDown(int b) const { return b >= 0 && b < MOUSE_BUTTON_COUNT ? m_Mouse[b] : false; }
    bool Input::IsMousePressed(int b) const { return b >= 0 && b < MOUSE_BUTTON_COUNT ? (m_Mouse[b] && !m_PrevMouse[b]) : false; }
    bool Input::IsMouseReleased(int b) const { return b >= 0 && b < MOUSE_BUTTON_COUNT ? (!m_Mouse[b] && m_PrevMouse[b]) : false; }

} // namespace ar