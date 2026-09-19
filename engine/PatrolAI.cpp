#include "PatrolAI.h"
#include "Entity.h"
#include "RigidBody.h"

namespace ar {

    void PatrolAI::Update(float dt) {
        (void)dt;
        Entity* owner = GetOwner();
        if (!owner || !owner->Active) return;

        auto* rb = owner->GetComponent<RigidBody>();
        if (rb) rb->Velocity.x = Direction * Speed;

        auto* t = owner->GetComponent<Transform>();
        if (!t) return;

        if (t->Position.x <= MinX) Direction = 1.0f;
        if (t->Position.x >= MaxX) Direction = -1.0f;
    }

} // namespace ar
