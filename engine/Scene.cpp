#include "Scene.h"
#include "Entity.h"
#include "Component.h"
#include "RigidBody.h"
#include "Gravity.h"
#include "Collision.h"
#include <algorithm>

namespace ar {

    Scene::~Scene() { Clear(); }

    Entity* Scene::CreateEntity(const std::string& name, const std::string& tag) {
        auto e = std::make_unique<Entity>(name, tag);
        Entity* ptr = e.get();
        m_Entities.push_back(std::move(e));
        return ptr;
    }

    void Scene::DestroyEntity(Entity* entity) {
        if (entity) m_ToDestroy.push_back(entity);
    }

    void Scene::Update(float dt) {
        // Reset ground state and apply gravity to all rigidbodies
        for (auto& e : m_Entities) {
            if (auto* rb = e->GetComponent<RigidBody>()) {
                rb->ResetGroundState();
                Gravity::Apply(rb);
            }
        }

        // Integrate positions and velocities
        for (auto& e : m_Entities) e->Update(dt);

        Collision::BeginTriggerFrame();
        for (size_t i = 0; i < m_Entities.size(); ++i)
            for (size_t j = i + 1; j < m_Entities.size(); ++j)
                Collision::CheckAndResolve(m_Entities[i].get(), m_Entities[j].get());
        Collision::EndTriggerFrame();
        if (!m_ToDestroy.empty()) {
            m_Entities.erase(
                std::remove_if(m_Entities.begin(), m_Entities.end(),
                    [this](const std::unique_ptr<Entity>& e) {
                        return std::find(m_ToDestroy.begin(), m_ToDestroy.end(), e.get()) != m_ToDestroy.end();
                    }), m_Entities.end());
            m_ToDestroy.clear();
        }
    }

    void Scene::Render(Renderer* renderer) {
        struct DrawItem {
            int layer;
            Entity* entity;
        };
        std::vector<DrawItem> order;
        order.reserve(m_Entities.size());
        for (auto& e : m_Entities) {
            if (!e->Active) continue;
            int layer = static_cast<int>(RenderLayer::Main);
            if (auto* sr = e->GetComponent<SpriteRenderer>())
                layer = static_cast<int>(sr->GetLayer());
            order.push_back({ layer, e.get() });
        }
        std::sort(order.begin(), order.end(),
            [](const DrawItem& a, const DrawItem& b) { return a.layer < b.layer; });
        for (const auto& item : order) item.entity->Render(renderer);
    }

    Entity* Scene::FindEntity(const std::string& name) {
        for (auto& e : m_Entities) if (e->Name == name) return e.get();
        return nullptr;
    }

    std::vector<Entity*> Scene::FindEntitiesByTag(const std::string& tag) {
        std::vector<Entity*> r;
        for (auto& e : m_Entities) if (e->Tag == tag) r.push_back(e.get());
        return r;
    }

    void Scene::Clear() {
        m_Entities.clear();
        m_ToDestroy.clear();
    }

} // namespace ar