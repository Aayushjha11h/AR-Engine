#pragma once
#include <glm/glm.hpp>

namespace ar {
    class Renderer;
    class Entity;
    class Texture;

    class Component {
    public:
        virtual ~Component() = default;
        virtual void OnAttach(Entity* owner) { m_Owner = owner; }
        virtual void Update(float dt) {}
        virtual void Render(Renderer* renderer) {}
        Entity* GetOwner() const { return m_Owner; }
    protected:
        Entity* m_Owner = nullptr;
    };

    struct Transform : public Component {
        glm::vec2 Position = { 0,0 };
        float Rotation = 0.0f;
        glm::vec2 Scale = { 1,1 };
    };

    class SpriteRenderer : public Component {
    public:
        SpriteRenderer(Texture* tex = nullptr, const glm::vec4& color = { 1,1,1,1 });
        void Render(Renderer* renderer) override;
        void SetTexture(Texture* tex) { m_Texture = tex; }
        void SetColor(const glm::vec4& c) { m_Color = c; }
        void SetSize(const glm::vec2& s) { m_Size = s; }
    private:
        Texture* m_Texture;
        glm::vec4 m_Color;
        glm::vec2 m_Size;
    };

} // namespace ar