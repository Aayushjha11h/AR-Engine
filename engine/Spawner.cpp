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
        m_Timer = Interval;

        if (MaxSpawns >= 0 && SpawnCount >= MaxSpawns) return;

        glm::vec2 pos = SpawnOffset;
        if (SpawnAtSelf) {
            if (auto* t = GetOwner()->GetComponent<Transform>())
                pos += t->Position;
        }
        SpawnCallback(PrefabName, pos);
        ++SpawnCount;
    }

}
