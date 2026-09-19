#pragma once
#include "Component.h"
#include <glm/glm.hpp>

namespace ar {

    class RigidBody : public Component {
    public:
        glm::vec2 Velocity;
        glm::vec2 Acceleration;
        glm::vec2 Force;

        float Mass;
        float InverseMass;
        float LinearDrag;      // Air resistance / damping
        float GravityScale;    // Multiplier on global gravity
        float Restitution;     // Bounciness (0 = none, 1 = perfect)
        float Friction;        // Ground friction (slows X when IsOnGround)

        bool IsKinematic;      // If true, forces/velocity are ignored
        bool UseGravity;     // If false, global gravity is not applied
        bool IsOnGround;     // Set by Collision system each frame

        RigidBody();

        void ApplyForce(const glm::vec2& force);
        void ApplyImpulse(const glm::vec2& impulse);
        void ApplyAcceleration(const glm::vec2& accel); // Helper: auto-multiplies by mass

        void SetMass(float mass);
        float GetMass() const { return Mass; }

        void Update(float dt) override;
        void ResetGroundState() { IsOnGround = false; }

    private:
        void Integrate(float dt);
    };

} // namespace ar