#pragma once
#include <vector>
#include <memory>
#include <string>
#include "Entity.h"

namespace ar {
    class Renderer;

    class Scene {
    public:
        ~Scene();
        Entity* CreateEntity(const std::string& name = "Entity", const std::string& tag = "");
        void DestroyEntity(Entity* entity);
        Entity* FindEntity(const std::string& name);
        std::vector<Entity*> FindEntitiesByTag(const std::string& tag);
        void Update(float dt);
        void Render(Renderer* renderer);
        void Clear();

    private:
        std::vector<std::unique_ptr<Entity>> m_Entities;
        std::vector<Entity*> m_ToDestroy;
    };

} // namespace ar