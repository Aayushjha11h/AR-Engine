#include "Collision.h"
#include <cmath>
#include "Entity.h"
#include "Component.h" // Transform
#include "RigidBody.h"

namespace ar {

    AABB::AABB(const glm::vec2& min, const glm::vec2& max)
        : Min(min), Max(max) {
    }

    glm::vec2 AABB::GetCenter() const {
        return (Min + Max) * 0.5f;
    }

    glm::vec2 AABB::GetHalfSize() const {
        return (Max - Min) * 0.5f;
    }

    AABB Collider::GetWorldBounds() const {
        Transform* t = GetOwner()->GetComponent<Transform>();
        glm::vec2 pos = t ? t->Position : glm::vec2(0.0f);
        glm::vec2 scl = t ? t->Scale : glm::vec2(1.0f);

        glm::vec2 worldPos = pos + Offset;
        glm::vec2 halfSize = Size * scl * 0.5f;

        return AABB(worldPos - halfSize, worldPos + halfSize);
    }

    bool Collision::AABBvsAABB(const AABB& a, const AABB& b) {
        return (a.Min.x <= b.Max.x && a.Max.x >= b.Min.x) &&
            (a.Min.y <= b.Max.y && a.Max.y >= b.Min.y);
    }

    bool Collision::AABBvsAABB(const AABB& a, const AABB& b, CollisionManifold& out) {
        if (!AABBvsAABB(a, b)) {
            out.Colliding = false;
            return false;
        }

        glm::vec2 n = b.GetCenter() - a.GetCenter();
        glm::vec2 overlap = (a.GetHalfSize() + b.GetHalfSize()) - glm::abs(n);

        if (overlap.x < overlap.y) {
            out.Normal = (n.x < 0.0f) ? glm::vec2(-1.0f, 0.0f) : glm::vec2(1.0f, 0.0f);
            out.Penetration = overlap.x;
        }
        else {
            out.Normal = (n.y < 0.0f) ? glm::vec2(0.0f, -1.0f) : glm::vec2(0.0f, 1.0f);
            out.Penetration = overlap.y;
        }

        out.Colliding = true;
        return true;
    }

    void Collision::Resolve(Entity* a, Entity* b, const CollisionManifold& m) {
        if (!m.Colliding) return;

        Collider* ca = a->GetComponent<Collider>();
        Collider* cb = b->GetComponent<Collider>();
        if (!ca || !cb) return;

        // Layer mask check
        if (!(ca->Mask & cb->Layer) || !(cb->Mask & ca->Layer)) return;

        // Trigger-only: skip physics resolution
        if (ca->IsTrigger || cb->IsTrigger) return;

        Transform* ta = a->GetComponent<Transform>();
        Transform* tb = b->GetComponent<Transform>();
        if (!ta || !tb) return;

        RigidBody* rba = a->GetComponent<RigidBody>();
        RigidBody* rbb = b->GetComponent<RigidBody>();

        bool aStatic = !rba || rba->IsKinematic;
        bool bStatic = !rbb || rbb->IsKinematic;

        if (aStatic && bStatic) return;

        // --- Positional correction (split based on movability) ---
        const float percent = 0.5f;
        const float slop = 0.01f;
        glm::vec2 correction = m.Normal * (m.Penetration - slop);

        if (aStatic) {
            tb->Position -= correction;
        }
        else if (bStatic) {
            ta->Position += correction;
        }
        else {
            ta->Position += correction * percent;
            tb->Position -= correction * percent;
        }

        // --- Impulse resolution ---
        glm::vec2 rv = (rbb ? rbb->Velocity : glm::vec2(0.0f))
            - (rba ? rba->Velocity : glm::vec2(0.0f));

        float velAlongNormal = glm::dot(rv, m.Normal);
        if (velAlongNormal > 0.0f) return; // separating

        float e = std::min(
            rba ? rba->Restitution : 0.0f,
            rbb ? rbb->Restitution : 0.0f
        );

        float invMassA = rba ? rba->InverseMass : 0.0f;
        float invMassB = rbb ? rbb->InverseMass : 0.0f;
        float massSum = invMassA + invMassB;
        if (massSum == 0.0f) return;

        float j = -(1.0f + e) * velAlongNormal;
        j /= massSum;

        glm::vec2 impulse = j * m.Normal;
        if (rba) rba->ApplyImpulse(-impulse);
        if (rbb) rbb->ApplyImpulse(impulse);

        // --- Friction ---
        rv = (rbb ? rbb->Velocity : glm::vec2(0.0f))
            - (rba ? rba->Velocity : glm::vec2(0.0f));

        glm::vec2 tangent = rv - (glm::dot(rv, m.Normal) * m.Normal);
        if (glm::length(tangent) > 0.0001f)
            tangent = glm::normalize(tangent);

        float jt = -glm::dot(rv, tangent);
        jt /= massSum;

        float mu = std::sqrt(
            (rba ? rba->Friction : 0.0f) * (rbb ? rbb->Friction : 0.0f)
        );

        glm::vec2 frictionImpulse = (std::abs(jt) < j * mu)
            ? tangent * jt
            : tangent * (-j * mu);

        if (rba) rba->ApplyImpulse(-frictionImpulse);
        if (rbb) rbb->ApplyImpulse(frictionImpulse);

        // --- Ground detection for RigidBody ---
        if (rba && m.Normal.y > 0.5f) rba->IsOnGround = true;
        if (rbb && m.Normal.y < -0.5f) rbb->IsOnGround = true;
    }

    void Collision::CheckAndResolve(Entity* a, Entity* b) {
        if (!a || !b || a == b) return;
        if (!a->Active || !b->Active) return;

        Collider* ca = a->GetComponent<Collider>();
        Collider* cb = b->GetComponent<Collider>();
        if (!ca || !cb) return;

        CollisionManifold m;
        if (AABBvsAABB(ca->GetWorldBounds(), cb->GetWorldBounds(), m)) {
            Resolve(a, b, m);
        }
    }

} // namespace ar