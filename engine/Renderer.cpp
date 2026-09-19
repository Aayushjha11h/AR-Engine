#include "Renderer.h"
#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>

namespace ar {

    static const char* vertSrc = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTex;
out vec2 TexCoord;
out vec4 VertexColor;
uniform mat4 uProjection;
uniform mat4 uModel;
uniform vec4 uColor;
void main() {
    gl_Position = uProjection * uModel * vec4(aPos, 0.0, 1.0);
    TexCoord = aTex;
    VertexColor = uColor;
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
        return true;
    }

    void Renderer::CreateQuad() {
        float verts[] = {
            // pos      // tex
            0.0f, 0.0f, 0.0f, 0.0f,
            1.0f, 0.0f, 1.0f, 0.0f,
            1.0f, 1.0f, 1.0f, 1.0f,
            0.0f, 1.0f, 0.0f, 1.0f
        };
        unsigned int idx[] = { 0, 1, 2, 0, 2, 3 };

        glGenVertexArrays(1, &m_VAO);
        glGenBuffers(1, &m_VBO);
        glGenBuffers(1, &m_EBO);

        glBindVertexArray(m_VAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

        glBindVertexArray(0);
    }

    void Renderer::Shutdown() {
        glDeleteVertexArrays(1, &m_VAO);
        glDeleteBuffers(1, &m_VBO);
        glDeleteBuffers(1, &m_EBO);
    }

    void Renderer::BeginScene(const glm::mat4& viewProjection) {
        m_Projection = viewProjection;
        m_Shader.Use();
        m_Shader.SetMat4("uProjection", m_Projection);
    }
    

    void Renderer::Clear() {
        glClearColor(m_ClearColor.r, m_ClearColor.g, m_ClearColor.b, m_ClearColor.a);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    void Renderer::DrawQuad(const glm::vec2& pos, const glm::vec2& size, const glm::vec4& color) {
        Sprite s; s.Position = pos; s.Size = size; s.Color = color; s.Tex = nullptr;
        DrawSprite(s);
    }

    void Renderer::DrawSprite(const Sprite& sprite) {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(sprite.Position, 0.0f));
        model = glm::scale(model, glm::vec3(sprite.Size, 1.0f));

        m_Shader.Use();
        m_Shader.SetMat4("uModel", model);
        m_Shader.SetVec4("uColor", sprite.Color);

        if (sprite.Tex && sprite.Tex->GetID()) {
            m_Shader.SetInt("uUseTexture", 1);
            sprite.Tex->Bind(0);
        }
        else {
            m_Shader.SetInt("uUseTexture", 0);
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        glBindVertexArray(m_VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);
    }

    void Renderer::EndScene() {}

} // namespace ar