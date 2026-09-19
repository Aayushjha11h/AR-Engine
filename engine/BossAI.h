#pragma once
#include "Component.h"

namespace ar {

    class Entity;

    class BossAI : public Component {
    public:
        int Health = 3;
        int MaxHealth = 3;
        float InvulnTime = 0.0f;
        float JumpCooldown = 0.0f;
        float ChargeCooldown = 0.0f;
        float Phase2SpeedMult = 1.6f;
        Entity* PlayerTarget = nullptr;

        void Update(float dt) override;
        bool TakeStompDamage();
        bool IsDefeated() const { return Health <= 0; }

    private:
        void UpdatePhase1(float dt, Entity* player);
        void UpdatePhase2(float dt, Entity* player);
        void FlashInvuln(float dt);
    };

} // namespace ar
