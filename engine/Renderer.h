#pragma once
#include <glm/glm.hpp>
#include "Shader.h"
#include "Sprite.h"
#include "Texture.h"

namespace ar {

    class Renderer {
    public:
        bool Init();
        void Shutdown();

        void BeginScene(const glm::mat4& viewProjection);  // was: projection
        void EndScene();

        void DrawQuad(const glm::vec2& pos, const glm::vec2& size, const glm::vec4& color);
        void DrawSprite(const Sprite& sprite);
        void DrawParallaxBackground(const glm::vec2& cameraPos, float viewWidth, float viewHeight,
            Texture* texture, float parallaxFactor, const glm::vec4& fallbackColor);
        void DrawScreenOverlay(const glm::vec2& cameraPos, float viewWidth, float viewHeight,
            const glm::vec4& color);

        void Clear();
        void SetClearColor(const glm::vec4& c) { m_ClearColor = c; }

    private:
        unsigned int m_VAO = 0, m_VBO = 0, m_EBO = 0;
        Shader m_Shader;
        glm::mat4 m_Projection;
        glm::vec4 m_ClearColor = { 0.06f, 0.06f, 0.08f, 1.0f };

        void CreateQuad();
    };

} // namespace ar