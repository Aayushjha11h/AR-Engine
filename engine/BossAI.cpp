#include "BossAI.h"
#include "Entity.h"
#include "RigidBody.h"
#include "Component.h"
#include <cmath>
#include <algorithm>

namespace ar {

    bool BossAI::TakeStompDamage() {
        if (InvulnTime > 0.0f || Health <= 0) return false;
        --Health;
        InvulnTime = 0.6f;
        return true;
    }

    void BossAI::FlashInvuln(float dt) {
        if (InvulnTime <= 0.0f) return;
        InvulnTime -= dt;
        auto* sr = GetOwner()->GetComponent<SpriteRenderer>();
        if (!sr) return;
        bool flash = static_cast<int>(InvulnTime * 10.0f) % 2 == 0;
        sr->SetColor(flash ? glm::vec4(1.0f, 0.2f, 0.2f, 1.0f) : glm::vec4(0.9f, 0.2f, 0.9f, 1.0f));
    }

    void BossAI::UpdatePhase1(float dt, Entity* player) {
        auto* rb = GetOwner()->GetComponent<RigidBody>();
        auto* t = GetOwner()->GetComponent<Transform>();
        auto* pt = player ? player->GetComponent<Transform>() : nullptr;
        if (!rb || !t || !pt) return;

        JumpCooldown -= dt;
        if (JumpCooldown <= 0.0f && rb->IsOnGround) {
            rb->Velocity.y = -420.0f;
            rb->IsOnGround = false;
            JumpCooldown = 2.2f;
        }

        float dx = pt->Position.x - t->Position.x;
        rb->Velocity.x = (dx > 0.0f ? 1.0f : -1.0f) * 60.0f;
    }

    void BossAI::UpdatePhase2(float dt, Entity* player) {
        auto* rb = GetOwner()->GetComponent<RigidBody>();
        auto* t = GetOwner()->GetComponent<Transform>();
        auto* pt = player ? player->GetComponent<Transform>() : nullptr;
        if (!rb || !t || !pt) return;

        ChargeCooldown -= dt;
        float chargeSpeed = 140.0f * Phase2SpeedMult;
        if (ChargeCooldown <= 0.0f) {
            float dx = pt->Position.x - t->Position.x;
            rb->Velocity.x = (dx > 0.0f ? 1.0f : -1.0f) * chargeSpeed;
            if (std::abs(dx) < 24.0f) ChargeCooldown = 1.4f;
        }
        else {
            rb->Velocity.x *= 0.92f;
        }
    }

    void BossAI::Update(float dt) {
        if (Health <= 0) return;

        FlashInvuln(dt);

        if (Health <= 2) UpdatePhase2(dt, PlayerTarget);
        else UpdatePhase1(dt, PlayerTarget);
    }

} // namespace ar
