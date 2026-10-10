#include "PatrolAI.h"
#include "Entity.h"
#include "RigidBody.h"

namespace ar {

    void PatrolAI::Update(float dt) {
        Entity* owner = GetOwner();
        if (!owner || !owner->Active) return;

        // Yield to chase driver while its timeout is running.
        if (Chasing) {
            ChaseTimeout -= dt;
            if (ChaseTimeout > 0.0f) return;
            Chasing = false;
            ChaseTimeout = 0.0f;
        }

        auto* rb = owner->GetComponent<RigidBody>();
        if (rb) rb->Velocity.x = Direction * Speed;

        auto* t = owner->GetComponent<Transform>();
        if (!t) return;

        if (t->Position.x <= MinX) Direction = 1.0f;
        if (t->Position.x >= MaxX) Direction = -1.0f;
    }

} // namespace ar