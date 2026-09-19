#pragma once
#include <glm/glm.hpp>

namespace ar {

    class RigidBody;

    struct Gravity {
        static glm::vec2 Force; // pixels/sec^2, default (0, -980)

        static void Apply(RigidBody* rb);
    };

} // namespace ar