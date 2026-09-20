#pragma once
#include <glm/glm.hpp>
#include <algorithm>

namespace ar {

    class Camera {
    public:
        Camera(float width, float height);

        void SetPosition(const glm::vec2& pos) { m_Position = pos; Update(); }
        void Move(const glm::vec2& delta) { m_Position += delta; Update(); }
        void SetZoom(float z) { m_Zoom = std::clamp(z, 0.1f, 10.0f); Update(); }
        void Zoom(float amount) { m_Zoom = std::clamp(m_Zoom + amount, 0.1f, 10.0f); Update(); }

        void Follow(const glm::vec2& target, float smoothSpeed, float dt);

        glm::mat4 GetViewProjection() const;
        glm::vec2 ScreenToWorld(const glm::vec2& screen) const;

        glm::vec2 GetPosition() const { return m_Position; }
        float GetZoom() const { return m_Zoom; }

        void Shake(float amplitude, float decay = 6.0f, float duration = 0.5f);
        void UpdateShake(float dt);
        glm::vec2 GetShakeOffset() const { return m_ShakeOffset; }

    private:
        glm::vec2 m_Position;
        float m_Zoom, m_Width, m_Height;
        glm::mat4 m_Projection, m_View;

        float m_ShakeAmp = 0.0f;
        float m_ShakeDecay = 6.0f;
        float m_ShakeDuration = 0.0f;
        glm::vec2 m_ShakeOffset{0.0f, 0.0f};

        void Update();
    };

} // namespace ar