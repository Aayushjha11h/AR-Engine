#pragma once
#include <glm/glm.hpp>
#include <vector>
#include "Shader.h"
#include "Sprite.h"
#include "Texture.h"

namespace ar {

    class Renderer {
    public:
        bool Init();
        void Shutdown();

        void BeginScene(const glm::mat4& viewProjection);
        void EndScene();

        void DrawQuad(const glm::vec2& pos, const glm::vec2& size, const glm::vec4& color);
        void DrawSprite(const Sprite& sprite);

        void DrawParallaxBackground(const glm::vec2& cameraPos, float viewWidth, float viewHeight,
                                    Texture* texture, float parallaxFactor,
                                    const glm::vec4& fallbackColor);
        void DrawScreenOverlay(const glm::vec2& cameraPos, float viewWidth, float viewHeight,
                               const glm::vec4& color);

        void Clear();
        void SetClearColor(const glm::vec4& c) { m_ClearColor = c; }

        // -------- Batch API --------
        void BeginBatch();
        void SubmitSprite(const Sprite& sprite);
        void FlushBatch();

    private:
        unsigned int m_VAO = 0, m_VBO = 0, m_EBO = 0;
        Shader m_Shader;
        glm::mat4 m_Projection;
        glm::vec4 m_ClearColor = { 0.06f, 0.06f, 0.08f, 1.0f };

        // Batch state
        std::vector<float>        m_BatchVertexData;   // 8 floats per vertex
        std::vector<unsigned int> m_BatchIndexData;
        struct BatchGroup {
            Texture*     Tex;
            unsigned int FirstIndex;
            unsigned int IndexCount;
        };
        std::vector<BatchGroup> m_BatchGroups;

        void CreateQuad();
    };

}
