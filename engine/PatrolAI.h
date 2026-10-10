#pragma once
#include "Component.h"

namespace ar {

    class PatrolAI : public Component {
    public:
        float Speed = 80.0f;
        float Direction = -1.0f;
        float MinX = 0.0f;
        float MaxX = 0.0f;

        // When another system (e.g. move_toward) drives this entity's velocity,
        // it sets Chasing = true and ChaseTimeout = seconds. PatrolAI skips
        // while the timeout is running, then resumes automatic patrol.
        bool  Chasing = false;
        float ChaseTimeout = 0.0f;

        void Update(float dt) override;
    };

} // namespace ar