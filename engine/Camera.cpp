#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <cstdlib>

namespace ar {

    Camera::Camera(float w, float h)
        : m_Position(0, 0), m_Zoom(1.0f), m_Width(w), m_Height(h) { Update(); }

    void Camera::Update() {
        m_Projection = glm::ortho(-m_Width * 0.5f * m_Zoom, m_Width * 0.5f * m_Zoom,
                                   m_Height * 0.5f * m_Zoom, -m_Height * 0.5f * m_Zoom,
                                   -1.0f, 1.0f);
        m_View = glm::translate(glm::mat4(1.0f),
                                glm::vec3(-(m_Position + m_ShakeOffset), 0.0f));
    }

    glm::mat4 Camera::GetViewProjection() const {
        return m_Projection * m_View;
    }

    void Camera::Follow(const glm::vec2& target, float s, float dt) {
        m_Position += (target - m_Position) * s * dt;
        Update();
    }

    glm::vec2 Camera::ScreenToWorld(const glm::vec2& s) const {
        return { m_Position.x + (s.x - m_Width * 0.5f) * m_Zoom,
                 m_Position.y + (s.y - m_Height * 0.5f) * m_Zoom };
    }

    void Camera::Shake(float amplitude, float decay, float duration) {
        m_ShakeAmp      = std::max(m_ShakeAmp, amplitude);
        m_ShakeDecay    = decay;
        m_ShakeDuration = std::max(m_ShakeDuration, duration);
    }

    void Camera::UpdateShake(float dt) {
        if (m_ShakeDuration <= 0.0f || m_ShakeAmp <= 0.001f) {
            if (m_ShakeOffset.x != 0.0f || m_ShakeOffset.y != 0.0f) {
                m_ShakeAmp = 0.0f;
                m_ShakeOffset = { 0.0f, 0.0f };
                Update();
            }
            return;
        }

        m_ShakeDuration -= dt;
        m_ShakeAmp      -= m_ShakeDecay * dt;
        if (m_ShakeAmp < 0.0f) m_ShakeAmp = 0.0f;

        if (m_ShakeDuration <= 0.0f || m_ShakeAmp <= 0.0f) {
            m_ShakeOffset = { 0.0f, 0.0f };
            Update();
            return;
        }

        float rx = ((std::rand() % 2000) / 1000.0f - 1.0f) * m_ShakeAmp;
        float ry = ((std::rand() % 2000) / 1000.0f - 1.0f) * m_ShakeAmp;
        m_ShakeOffset = { rx, ry };
        Update();
    }

}
