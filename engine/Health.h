#pragma once
#include "Component.h"
#include <glm/glm.hpp>

namespace ar {

    class Health : public Component {
    public:
        int Current = 1;
        int Max = 1;
        float InvulnTime = 0.0f;
        float InvulnDuration = 0.3f;
        float KnockbackForce = 0.0f;                      // 0 = disabled
        glm::vec4 FlashColor  = { 1.0f, 0.3f, 0.3f, 1.0f };
        glm::vec4 NormalColor = { 1.0f, 1.0f, 1.0f, 1.0f };

        bool IsDead() const { return Current <= 0; }
        bool CanTakeDamage() const { return InvulnTime <= 0.0f && Current > 0; }

        // sourcePos: world position of damage source (used to compute knockback direction).
        // Pass a very far point or same pos to skip meaningful knockback.
        bool TakeDamage(int amount, const glm::vec2& sourcePos, float invulnAfter = -1.0f);
        void Heal(int amount);

        void Update(float dt) override;
        void OnAttach(Entity* owner) override;
    };

}
