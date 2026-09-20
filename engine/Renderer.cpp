#include "Renderer.h"
#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

namespace ar {

    static const char* vertSrc = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTex;
layout (location = 2) in vec4 aColor;
out vec2 TexCoord;
out vec4 VertexColor;
uniform mat4 uProjection;
void main() {
    gl_Position = uProjection * vec4(aPos, 0.0, 1.0);
    TexCoord    = aTex;
    VertexColor = aColor;
})";

    static const char* fragSrc = R"(
#version 330 core
in vec2 TexCoord;
in vec4 VertexColor;
out vec4 FragColor;
uniform sampler2D uTexture;
uniform int uUseTexture;
void main() {
    vec4 tex = (uUseTexture == 1) ? texture(uTexture, TexCoord) : vec4(1.0);
    FragColor = tex * VertexColor;
})";

    bool Renderer::Init() {
        if (!m_Shader.Load(vertSrc, fragSrc)) return false;
        CreateQuad();
        m_BatchVertexData.reserve(8192);
        m_BatchIndexData.reserve(6144);
        return true;
    }

    void Renderer::CreateQuad() {
        glGenVertexArrays(1, &m_VAO);
        glGenBuffers(1, &m_VBO);
        glGenBuffers(1, &m_EBO);

        glBindVertexArray(m_VAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);

        // 8 floats/vertex: pos(2), uv(2), color(4)
        const GLsizei stride = 8 * sizeof(float);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, stride, (void*)(4 * sizeof(float)));

        glBindVertexArray(0);
    }

    void Renderer::Shutdown() {
        glDeleteVertexArrays(1, &m_VAO);
        glDeleteBuffers(1, &m_VBO);
        glDeleteBuffers(1, &m_EBO);
    }

    void Renderer::BeginScene(const glm::mat4& viewProjection) {
        m_Projection = viewProjection;
        BeginBatch();
    }

    void Renderer::EndScene() {
        FlushBatch();
    }

    void Renderer::Clear() {
        glClearColor(m_ClearColor.r, m_ClearColor.g, m_ClearColor.b, m_ClearColor.a);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    void Renderer::BeginBatch() {
        m_BatchVertexData.clear();
        m_BatchIndexData.clear();
        m_BatchGroups.clear();
    }

    void Renderer::SubmitSprite(const Sprite& sprite) {
        glm::vec2 halfSize = sprite.Size * 0.5f;
        glm::vec2 corners[4] = {
            { -halfSize.x, -halfSize.y },
            {  halfSize.x, -halfSize.y },
            {  halfSize.x,  halfSize.y },
            { -halfSize.x,  halfSize.y }
        };

        float cosR = 1.0f, sinR = 0.0f;
        if (sprite.Rotation != 0.0f) {
            float rad = glm::radians(sprite.Rotation);
            cosR = std::cos(rad);
            sinR = std::sin(rad);
        }

        static const glm::vec2 uvs[4] = { {0,0}, {1,0}, {1,1}, {0,1} };

        // Start a new texture group when texture changes (preserves draw order).
        if (m_BatchGroups.empty() || m_BatchGroups.back().Tex != sprite.Tex) {
            BatchGroup g;
            g.Tex        = sprite.Tex;
            g.FirstIndex = static_cast<unsigned int>(m_BatchIndexData.size());
            g.IndexCount = 0;
            m_BatchGroups.push_back(g);
        }

        unsigned int vertexBase = static_cast<unsigned int>(m_BatchVertexData.size() / 8);

        for (int i = 0; i < 4; ++i) {
            glm::vec2 p = corners[i];
            glm::vec2 r = { p.x * cosR - p.y * sinR, p.x * sinR + p.y * cosR };
            glm::vec2 w = sprite.Position + r;

            m_BatchVertexData.push_back(w.x);
            m_BatchVertexData.push_back(w.y);
            m_BatchVertexData.push_back(uvs[i].x);
            m_BatchVertexData.push_back(uvs[i].y);
            m_BatchVertexData.push_back(sprite.Color.r);
            m_BatchVertexData.push_back(sprite.Color.g);
            m_BatchVertexData.push_back(sprite.Color.b);
            m_BatchVertexData.push_back(sprite.Color.a);
        }

        m_BatchIndexData.push_back(vertexBase + 0);
        m_BatchIndexData.push_back(vertexBase + 1);
        m_BatchIndexData.push_back(vertexBase + 2);
        m_BatchIndexData.push_back(vertexBase + 0);
        m_BatchIndexData.push_back(vertexBase + 2);
        m_BatchIndexData.push_back(vertexBase + 3);

        m_BatchGroups.back().IndexCount += 6;
    }

    void Renderer::FlushBatch() {
        if (m_BatchIndexData.empty()) return;

        m_Shader.Use();
        m_Shader.SetMat4("uProjection", m_Projection);

        glBindVertexArray(m_VAO);

        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferData(GL_ARRAY_BUFFER,
                     m_BatchVertexData.size() * sizeof(float),
                     m_BatchVertexData.data(), GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     m_BatchIndexData.size() * sizeof(unsigned int),
                     m_BatchIndexData.data(), GL_DYNAMIC_DRAW);

        for (const auto& g : m_BatchGroups) {
            if (g.Tex && g.Tex->GetID()) {
                m_Shader.SetInt("uUseTexture", 1);
                g.Tex->Bind(0);
            } else {
                m_Shader.SetInt("uUseTexture", 0);
                glBindTexture(GL_TEXTURE_2D, 0);
            }
            glDrawElements(GL_TRIANGLES, g.IndexCount, GL_UNSIGNED_INT,
                           (void*)(g.FirstIndex * sizeof(unsigned int)));
        }

        glBindVertexArray(0);

        m_BatchVertexData.clear();
        m_BatchIndexData.clear();
        m_BatchGroups.clear();
    }

    void Renderer::DrawQuad(const glm::vec2& pos, const glm::vec2& size, const glm::vec4& color) {
        Sprite s; s.Position = pos; s.Size = size; s.Color = color; s.Tex = nullptr;
        SubmitSprite(s);
    }

    void Renderer::DrawSprite(const Sprite& sprite) {
        SubmitSprite(sprite);
    }

    void Renderer::DrawParallaxBackground(const glm::vec2& cameraPos, float viewWidth, float viewHeight,
        Texture* texture, float parallaxFactor, const glm::vec4& fallbackColor) {
        float parallaxX = cameraPos.x * parallaxFactor;
        glm::vec2 center = { cameraPos.x - parallaxX, cameraPos.y };
        glm::vec2 size   = { viewWidth * 1.5f, viewHeight * 1.2f };

        if (texture && texture->GetID()) {
            Sprite s;
            s.Position = center;
            s.Size     = size;
            s.Color    = { 1.0f, 1.0f, 1.0f, 1.0f };
            s.Tex      = texture;
            SubmitSprite(s);
        } else {
            DrawQuad(center, size, fallbackColor);
            DrawQuad({ center.x, center.y + viewHeight * 0.35f },
                     { size.x, size.y * 0.25f }, { 0.15f, 0.45f, 0.85f, 1.0f });
        }
    }

    void Renderer::DrawScreenOverlay(const glm::vec2& cameraPos, float viewWidth, float viewHeight,
        const glm::vec4& color) {
        DrawQuad(cameraPos, { viewWidth, viewHeight }, color);
    }

} // namespace ar
