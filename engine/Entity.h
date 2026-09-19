#pragma once
#include <string>
#include <vector>
#include <typeindex>
#include <unordered_map>
#include <memory>
#include <type_traits>
#include "Component.h"

namespace ar {

    class Entity {
    public:
        std::string Name;
        std::string Tag;
        bool Active = true;

        Entity(const std::string& name = "Entity", const std::string& tag = "");
        ~Entity();

        template<typename T, typename... Args>
        T* AddComponent(Args&&... args) {
            static_assert(std::is_base_of_v<Component, T>, "T must derive from Component");
            auto comp = std::make_unique<T>(std::forward<Args>(args)...);
            T* ptr = comp.get();
            comp->OnAttach(this);
            m_Components[typeid(T)] = std::move(comp);
            m_ComponentList.push_back(ptr);
            return ptr;
        }

        template<typename T> T* GetComponent() {
            auto it = m_Components.find(typeid(T));
            return it != m_Components.end() ? static_cast<T*>(it->second.get()) : nullptr;
        }

        template<typename T> bool HasComponent() {
            return m_Components.find(typeid(T)) != m_Components.end();
        }

        void Update(float dt);
        void Render(Renderer* renderer);

    private:
        std::unordered_map<std::type_index, std::unique_ptr<Component>> m_Components;
        std::vector<Component*> m_ComponentList;
    };

} // namespace ar