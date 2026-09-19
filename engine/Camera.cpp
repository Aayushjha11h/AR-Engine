#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>

namespace ar {

    Camera::Camera(float w, float h) : m_Position(0, 0), m_Zoom(1.0f), m_Width(w), m_Height(h) { Update(); }

    void Camera::Update() {
        m_Projection = glm::ortho(-m_Width * 0.5f * m_Zoom, m_Width * 0.5f * m_Zoom,
            m_Height * 0.5f * m_Zoom, -m_Height * 0.5f * m_Zoom, -1.0f, 1.0f);
        m_View = glm::translate(glm::mat4(1.0f), glm::vec3(-m_Position, 0.0f));
    }

    void Camera::Follow(const glm::vec2& target, float s, float dt) {
        m_Position += (target - m_Position) * s * dt;
        Update();
    }

    glm::vec2 Camera::ScreenToWorld(const glm::vec2& s) const {
        return { m_Position.x + (s.x - m_Width * 0.5f) * m_Zoom,
                 m_Position.y + (s.y - m_Height * 0.5f) * m_Zoom };
    }

} // namespace ar