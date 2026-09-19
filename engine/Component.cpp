#include "Component.h"
#include "Entity.h"
#include "Texture.h"
#include "Renderer.h"
#include "Sprite.h"

namespace ar {

    SpriteRenderer::SpriteRenderer(Texture* tex, const glm::vec4& color)
        : m_Texture(tex), m_Color(color), m_Size(64.0f, 64.0f) {
    }

    void ar::SpriteRenderer::Render(Renderer* renderer) {
        if (!m_Owner) return;
        auto* transform = m_Owner->GetComponent<Transform>();
        if (!transform) return;

        Sprite sprite;
        sprite.Position = transform->Position;
        sprite.Size = m_Size * transform->Scale;
        sprite.Rotation = transform->Rotation;
        sprite.Color = m_Color;
        sprite.Tex = m_Texture;
        renderer->DrawSprite(sprite);
    }

} // namespace ar