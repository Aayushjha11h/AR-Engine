#include "RigidBody.h"

#include "Entity.h"
#include "Component.h"

namespace ar {

    RigidBody::RigidBody()
        : Velocity(0.0f, 0.0f)
        , Acceleration(0.0f, 0.0f)
        , Force(0.0f, 0.0f)
        , Mass(1.0f)
        , InverseMass(1.0f)
        , LinearDrag(0.01f)
        , GravityScale(1.0f)
        , Restitution(0.0f)
        , Friction(0.0f)
        , IsKinematic(false)
        , UseGravity(true)
        , IsOnGround(false)
    {
    }

    void RigidBody::SetMass(float mass) {
        Mass = mass;
        InverseMass = (mass > 0.0f) ? (1.0f / mass) : 0.0f;
    }

    void RigidBody::ApplyForce(const glm::vec2& force) {
        if (IsKinematic) return;
        Force += force;
    }

    void RigidBody::ApplyImpulse(const glm::vec2& impulse) {
        if (IsKinematic) return;
        Velocity += impulse * InverseMass;
    }

    void RigidBody::ApplyAcceleration(const glm::vec2& accel) {
        if (IsKinematic || Mass <= 0.0f) return;
        Force += accel * Mass;
    }

    void RigidBody::Update(float dt) {
        if (IsKinematic) return;
        Integrate(dt);
    }

    void RigidBody::Integrate(float dt) {
        if (dt <= 0.0f) return;

        // Safety clamp to prevent physics explosion on lag spikes
        const float maxDt = 1.0f / 20.0f;
        if (dt > maxDt) dt = maxDt;

        // Apply linear drag (velocity damping)
        Velocity *= (1.0f / (1.0f + LinearDrag * dt));

        // a = F / m
        Acceleration = Force * InverseMass;

        // Integrate velocity
        Velocity += Acceleration * dt;

        // Apply ground friction
        if (IsOnGround && Friction > 0.0f) {
            Velocity.x *= (1.0f / (1.0f + Friction * dt));
        }

        // Integrate position via Transform component
        Transform* transform = GetOwner()->GetComponent<Transform>();
        if (transform) {
            transform->Position += Velocity * dt;
        }

        // Reset force accumulator for next frame
        Force = glm::vec2(0.0f, 0.0f);

        // Ground state is refreshed by Collision system each frame
        IsOnGround = false;
    }

} // namespace ar