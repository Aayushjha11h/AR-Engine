#include "Health.h"
#include "Entity.h"
#include "RigidBody.h"
#include <cmath>

namespace ar {

    void Health::OnAttach(Entity* owner) {
        Component::OnAttach(owner);
        if (auto* sr = owner->GetComponent<SpriteRenderer>())
            NormalColor = sr->GetColor();
    }

    bool Health::TakeDamage(int amount, const glm::vec2& sourcePos, float invulnAfter) {
        if (!CanTakeDamage()) return false;
        if (amount <= 0) return false;

        Current -= amount;
        if (Current < 0) Current = 0;

        InvulnTime = (invulnAfter < 0.0f) ? InvulnDuration : invulnAfter;

        if (KnockbackForce > 0.0f) {
            if (auto* rb = GetOwner()->GetComponent<RigidBody>()) {
                if (auto* t = GetOwner()->GetComponent<Transform>()) {
                    glm::vec2 dir = t->Position - sourcePos;
                    float len = glm::length(dir);
                    if (len < 0.001f) dir = { 0.0f, -1.0f };
                    else              dir /= len;
                    dir.y = -std::fabs(dir.y) - 0.5f;   // ensure upward pop
                    rb->ApplyImpulse(glm::normalize(dir) * KnockbackForce);
                }
            }
        }
        return true;
    }

    void Health::Heal(int amount) {
        if (amount <= 0) return;
        Current += amount;
        if (Current > Max) Current = Max;
    }

    void Health::Update(float dt) {
        // Always tick invulnerability, whether or not a sprite is present.
        if (InvulnTime > 0.0f) InvulnTime -= dt;

        auto* sr = GetOwner() ? GetOwner()->GetComponent<SpriteRenderer>() : nullptr;
        if (!sr) return;

        if (InvulnTime > 0.0f) {
            bool flash = static_cast<int>(InvulnTime * 12.0f) % 2 == 0;
            sr->SetColor(flash ? FlashColor : NormalColor);
        } else {
            sr->SetColor(NormalColor);
        }
    }

}