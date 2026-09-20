#pragma once
#include "Component.h"
#include <string>
#include <functional>
#include <glm/glm.hpp>

namespace ar {

    class Spawner : public Component {
    public:
        std::string PrefabName;
        float Interval     = 5.0f;
        float InitialDelay = 0.0f;
        int   MaxSpawns    = -1;      // -1 = infinite
        int   SpawnCount   = 0;
        glm::vec2 SpawnOffset = { 0.0f, 0.0f };
        bool SpawnAtSelf = true;      // if true, offset added to owner position
        bool Active = true;

        // Provided by RuntimeBridge: (prefabName, worldPos)
        std::function<void(const std::string&, const glm::vec2&)> SpawnCallback;

        void Update(float dt) override;

    private:
        float m_Timer = 0.0f;
        bool  m_Started = false;
    };

}
