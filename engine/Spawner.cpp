#include "Spawner.h"
#include "Entity.h"

namespace ar {

    void Spawner::Update(float dt) {
        if (!Active || PrefabName.empty() || !SpawnCallback) return;

        if (!m_Started) {
            m_Timer = InitialDelay;
            m_Started = true;
        }

        m_Timer -= dt;
        if (m_Timer > 0.0f) return;

        if (MaxSpawns >= 0 && SpawnCount >= MaxSpawns) {
            m_Timer = Interval;
            return;
        }

        glm::vec2 pos = SpawnOffset;
        if (SpawnAtSelf) {
            if (auto* t = GetOwner()->GetComponent<Transform>())
                pos += t->Position;
        }
        // Only count a successful spawn; a missing prefab should not burn the quota
        // or spam the callback every frame.
        Entity* spawned = SpawnCallback(PrefabName, pos);
        m_Timer = Interval;
        if (spawned) ++SpawnCount;
    }

}
