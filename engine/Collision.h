#pragma once
#include <glm/glm.hpp>
#include <functional>
#include "Component.h"

namespace ar {

    class Entity;

    struct AABB {
        glm::vec2 Min;
        glm::vec2 Max;

        AABB() = default;
        AABB(const glm::vec2& min, const glm::vec2& max);

        glm::vec2 GetCenter() const;
        glm::vec2 GetHalfSize() const;
    };

    struct CollisionManifold {
        bool Colliding = false;
        glm::vec2 Normal = { 0.0f, 0.0f };
        float Penetration = 0.0f;
    };

    using CollisionCallback = std::function<void(Entity* self, Entity* other, const CollisionManifold& m)>;
    using TriggerCallback = std::function<void(Entity* self, Entity* other)>;

    class Collider : public Component {
    public:
        glm::vec2 Offset = { 0.0f, 0.0f };
        glm::vec2 Size = { 1.0f, 1.0f }; // width, height in world units
        bool IsTrigger = false;
        int Layer = 1;          // what layer this collider lives on
        int Mask = ~0;          // layers it can collide with

        CollisionCallback OnCollision;
        TriggerCallback OnTriggerEnter;

        AABB GetWorldBounds() const;
    };

    namespace Collision {

        bool AABBvsAABB(const AABB& a, const AABB& b);
        bool AABBvsAABB(const AABB& a, const AABB& b, CollisionManifold& out);

        // Positional + impulse resolution. Uses RigidBody if present.
        void Resolve(Entity* a, Entity* b, const CollisionManifold& m);

        // Convenience: bounds check + layer mask + resolve in one call
        void CheckAndResolve(Entity* a, Entity* b);

        void BeginTriggerFrame();
        void EndTriggerFrame();

    } // namespace Collision

} // namespace ar