#include "Entity.h"

namespace ar {

    Entity::Entity(const std::string& name, const std::string& tag) : Name(name), Tag(tag) {}
    Entity::~Entity() = default;

    void Entity::Update(float dt) {
        if (!Active) return;
        for (auto* c : m_ComponentList) c->Update(dt);
    }

    void Entity::Render(Renderer* renderer) {
        if (!Active) return;
        for (auto* c : m_ComponentList) c->Render(renderer);
    }

} // namespace ar