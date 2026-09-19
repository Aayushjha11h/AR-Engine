#include "Gravity.h"
#include "RigidBody.h"

namespace ar {

    glm::vec2 Gravity::Force = glm::vec2(0.0f, 980.0f);

    void Gravity::Apply(RigidBody* rb) {
        if (!rb) return;
        if (rb->IsKinematic) return;
        if (!rb->UseGravity) return;
        if (rb->GetMass() <= 0.0f) return;

        // F = m * g * scale
        rb->ApplyForce(Force * rb->GetMass() * rb->GravityScale);
    }

} // namespace ar