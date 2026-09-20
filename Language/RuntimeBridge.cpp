#include "RuntimeBridge.h"
#include "../engine/Camera.h"
#include "../engine/PatrolAI.h"
#include "../engine/BossAI.h"
#include <SDL2/SDL.h>
#include <iostream>
#include <sstream>
#include <cmath>
#include <algorithm>

namespace ar {

    RuntimeBridge::RuntimeBridge(Scene* scene, Input* input, Audio* audio, Camera* camera)
        : scene(scene), input(input), audio(audio), m_Camera(camera) {
    }

    uint64_t RuntimeBridge::PairKeyByName(const std::string& a, const std::string& b) {
        uint64_t ha = 0, hb = 0;
        for (char c : a) ha = ha * 131 + static_cast<unsigned char>(c);
        for (char c : b) hb = hb * 131 + static_cast<unsigned char>(c);
        if (ha > hb) std::swap(ha, hb);
        return (ha << 32) | (hb & 0xFFFFFFFFULL);
    }

    bool RuntimeBridge::StartsWith(const std::string& s, const std::string& prefix) {
        if (s.size() < prefix.size()) return false;
        for (size_t i = 0; i < prefix.size(); ++i) if (s[i] != prefix[i]) return false;
        return true;
    }

    bool RuntimeBridge::LooksLikeNumber(const std::string& s) {
        if (s.empty()) return false;
        size_t i = 0;
        if (s[0] == '-' || s[0] == '+') i = 1;
        if (i >= s.size()) return false;
        bool sawDigit = false, sawDot = false;
        for (; i < s.size(); ++i) {
            char c = s[i];
            if (c >= '0' && c <= '9') { sawDigit = true; continue; }
            if (c == '.' && !sawDot) { sawDot = true; continue; }
            return false;
        }
        return sawDigit;
    }

    bool RuntimeBridge::Load(const std::string& argdlSource) {
        scene->Clear();
        entities.clear();
        playerEntity = nullptr;
        textures.clear();
        sounds.clear();
        error.clear();
        m_CoinsCollected = 0;
        m_VictoryTriggered = false;
        m_PlayerDead = false;
        m_BackgroundTexture = nullptr;
        m_Prefabs.clear();
        m_PendingCollisions.clear();
        m_ContextEntity = nullptr;
        m_SpawnCounter = 0;
        m_CollisionPairsLast.clear();
        m_CollisionPairsThis.clear();
        m_TimerAccum.clear();
        m_AfterFired.clear();

        Lexer lexer(argdlSource);
        Parser parser(lexer);
        if (!parser.Parse()) {
            error = "Parse error: " + parser.GetError();
            return false;
        }

        if (!interpreter.Load(parser.GetAST())) {
            error = "Interpreter error: " + interpreter.GetError();
            return false;
        }

        std::cout << "[RuntimeBridge] Building objects from "
            << interpreter.GetObjects().size() << " script objects\n";

        if (!BuildObjects()) return false;
        return true;
    }

    void RuntimeBridge::Update(float dt) {
        m_MovedHorizontalThisFrame = false;
        m_MovedVerticalThisFrame = false;

        for (const auto& scriptEvent : interpreter.GetEvents()) {
            const std::string& trigger = scriptEvent.trigger;
            if (trigger.size() >= 8 && trigger.substr(0, 8) == "collide ") continue;
            if (trigger.size() >= 6 && trigger.substr(0, 6) == "timer ") continue;
            if (trigger.size() >= 6 && trigger.substr(0, 6) == "after ") continue;

            int scancode = StringToScancode(trigger);
            if (scancode < 0) continue;

            bool isJumpEvent = false;
            for (const auto& cmd : scriptEvent.commands) {
                if (cmd == "jump") { isJumpEvent = true; break; }
            }

            bool shouldFire = isJumpEvent ? input->IsKeyPressed(scancode)
                : input->IsKeyDown(scancode);

            if (shouldFire) {
                ExecuteCommands(scriptEvent.commands, playerEntity);
            }
        }

        ProcessTimerEvents(dt);
        ProcessCollisionEvents();

        {
            std::vector<Entity*> toKill;
            for (const auto& [n, ent] : entities) {
                if (!ent) continue;
                auto* p = ent->GetComponent<Projectile>();
                if (!p) continue;
                if (p->Lifetime <= 0.0f || p->HitSomething) toKill.push_back(ent);
            }
            for (Entity* e : toKill) DestroyEntitySafe(e);
        }

        m_CollisionPairsLast = std::move(m_CollisionPairsThis);
        m_CollisionPairsThis.clear();

        if (playerEntity) {
            auto* rb = playerEntity->GetComponent<RigidBody>();
            if (rb) {
                if (!m_MovedHorizontalThisFrame) {
                    rb->Velocity.x = 0.0f;
                }
                if (!rb->UseGravity && !m_MovedVerticalThisFrame) {
                    rb->Velocity.y = 0.0f;
                }
            }
        }
    }

    Entity* RuntimeBridge::GetEntity(const std::string& name) const {
        auto it = entities.find(name);
        if (it != entities.end()) return it->second;
        return scene->FindEntity(name);
    }

    int RuntimeBridge::GetPlayerHealth() const {
        if (!playerEntity) return 0;
        if (auto* h = playerEntity->GetComponent<Health>()) return h->Current;
        return 0;
    }

    int RuntimeBridge::GetPlayerMaxHealth() const {
        if (!playerEntity) return 0;
        if (auto* h = playerEntity->GetComponent<Health>()) return h->Max;
        return 0;
    }

    Entity* RuntimeBridge::ResolveTarget(const std::string& name) {
        if (name.empty()) return m_ContextEntity;
        Entity* e = GetEntity(name);
        return e ? e : m_ContextEntity;
    }

    static glm::vec4 TypeColor(const std::string& type) {
        if (type == "Character") return { 1.0f, 0.2f, 0.2f, 1.0f };
        if (type == "Platform")  return { 0.2f, 0.8f, 0.3f, 1.0f };
        if (type == "Item")      return { 1.0f, 0.9f, 0.2f, 1.0f };
        if (type == "Enemy")     return { 0.9f, 0.2f, 0.9f, 1.0f };
        return { 1.0f, 1.0f, 1.0f, 1.0f };
    }

    static glm::vec4 ParseColorName(const std::string& name) {
        if (name == "red")    return { 1.0f, 0.2f, 0.2f, 1.0f };
        if (name == "green")  return { 0.2f, 0.8f, 0.3f, 1.0f };
        if (name == "blue")   return { 0.2f, 0.4f, 1.0f, 1.0f };
        if (name == "yellow") return { 1.0f, 0.9f, 0.2f, 1.0f };
        if (name == "orange") return { 1.0f, 0.6f, 0.1f, 1.0f };
        if (name == "purple") return { 0.7f, 0.2f, 0.9f, 1.0f };
        if (name == "cyan")   return { 0.2f, 0.9f, 1.0f, 1.0f };
        if (name == "white")  return { 1.0f, 1.0f, 1.0f, 1.0f };
        if (name == "black")  return { 0.1f, 0.1f, 0.1f, 1.0f };
        if (name == "brown")  return { 0.6f, 0.4f, 0.2f, 1.0f };
        if (name == "pink")   return { 1.0f, 0.5f, 0.8f, 1.0f };
        return { 1.0f, 1.0f, 1.0f, 1.0f };
    }

    Entity* RuntimeBridge::BuildEntityFromScript(const std::string& name, const ScriptObject& scriptObj) {
        std::string entityTag;
        {
            auto itTag = scriptObj.properties.find("tag");
            if (itTag != scriptObj.properties.end()) {
                entityTag = itTag->second;
            }
            else if (scriptObj.type == "Prefab") {
                entityTag = scriptObj.name;
            }
            else {
                entityTag = scriptObj.type;
            }
        }

        Entity* entity = scene->CreateEntity(name, entityTag);
        entities[name] = entity;

        if (!playerEntity && (name == "Player" || scriptObj.type == "Character")) {
            playerEntity = entity;
            std::cout << "[RuntimeBridge] Set as player entity\n";
        }

        std::cout << "[RuntimeBridge] Creating entity: " << name
            << " of type " << scriptObj.type << " tag=" << entityTag << "\n";

        entity->AddComponent<Transform>();

        auto* sr = entity->AddComponent<SpriteRenderer>(nullptr);
        float width = 64.0f;
        float height = 64.0f;
        {
            auto itColor = scriptObj.properties.find("color");
            if (itColor != scriptObj.properties.end()) {
                sr->SetColor(ParseColorName(itColor->second));
            }
            else {
                sr->SetColor(TypeColor(scriptObj.type));
            }

            auto itSize = scriptObj.properties.find("size");
            if (itSize != scriptObj.properties.end()) {
                width = height = ParseFloat(itSize->second, 64.0f);
            }
            auto itW = scriptObj.properties.find("width");
            if (itW == scriptObj.properties.end()) itW = scriptObj.properties.find("w");
            if (itW != scriptObj.properties.end()) width = ParseFloat(itW->second, width);

            auto itH = scriptObj.properties.find("height");
            if (itH == scriptObj.properties.end()) itH = scriptObj.properties.find("h");
            if (itH != scriptObj.properties.end()) height = ParseFloat(itH->second, height);

            sr->SetSize(glm::vec2(width, height));
        }

        auto itSprite = scriptObj.properties.find("sprite");
        if (itSprite != scriptObj.properties.end()) {
            Texture* tex = GetOrLoadTexture(itSprite->second);
            if (tex) sr->SetTexture(tex);
        }

        if (scriptObj.type == "Background") {
            sr->SetLayer(RenderLayer::Background);
            sr->SkipCameraParallax = true;
            auto itBg = scriptObj.properties.find("sprite");
            if (itBg != scriptObj.properties.end())
                m_BackgroundTexture = GetOrLoadTexture(itBg->second);
        }

        auto itLayer = scriptObj.properties.find("layer");
        if (itLayer != scriptObj.properties.end()) {
            const std::string& layerName = itLayer->second;
            if (layerName == "background") sr->SetLayer(RenderLayer::Background);
            else if (layerName == "scenery") sr->SetLayer(RenderLayer::Scenery);
            else if (layerName == "foreground") sr->SetLayer(RenderLayer::Foreground);
            else sr->SetLayer(RenderLayer::Main);
        }

        auto itGravity = scriptObj.properties.find("gravity");
        bool wantsGravity = (itGravity != scriptObj.properties.end() && ParseBool(itGravity->second));

        auto itPhysics = scriptObj.properties.find("physics");
        bool physicsOff = (itPhysics != scriptObj.properties.end() &&
                          (itPhysics->second == "off" || itPhysics->second == "false"));

        bool wantsPhysics = !physicsOff && (wantsGravity ||
            scriptObj.type == "Character" ||
            scriptObj.type == "Enemy" ||
            scriptObj.type == "Item" ||
            scriptObj.properties.count("mass") ||
            scriptObj.properties.count("kinematic"));

        if (wantsPhysics) {
            auto* rb = entity->AddComponent<RigidBody>();
            rb->UseGravity = wantsGravity;

            auto itMass = scriptObj.properties.find("mass");
            if (itMass != scriptObj.properties.end()) {
                float m = ParseFloat(itMass->second, 1.0f);
                if (m > 0.0f) rb->SetMass(m);
            }

            auto itDrag = scriptObj.properties.find("linear_drag");
            if (itDrag != scriptObj.properties.end()) rb->LinearDrag = ParseFloat(itDrag->second, rb->LinearDrag);

            auto itRest = scriptObj.properties.find("restitution");
            if (itRest != scriptObj.properties.end()) rb->Restitution = ParseFloat(itRest->second, rb->Restitution);

            auto itFriction = scriptObj.properties.find("friction");
            if (itFriction != scriptObj.properties.end()) rb->Friction = ParseFloat(itFriction->second, rb->Friction);

            auto itGravScale = scriptObj.properties.find("gravity_scale");
            if (itGravScale != scriptObj.properties.end()) rb->GravityScale = ParseFloat(itGravScale->second, rb->GravityScale);

            auto itKinematic = scriptObj.properties.find("kinematic");
            if (itKinematic != scriptObj.properties.end()) rb->IsKinematic = ParseBool(itKinematic->second, false);
        }

        auto itHealth = scriptObj.properties.find("health");
        if (itHealth != scriptObj.properties.end()) {
            int hp = static_cast<int>(ParseFloat(itHealth->second, 1.0f));
            if (hp < 1) hp = 1;
            auto* h = entity->AddComponent<Health>();
            h->Current = hp;
            h->Max = hp;

            auto itInv = scriptObj.properties.find("invuln_duration");
            if (itInv != scriptObj.properties.end())
                h->InvulnDuration = ParseFloat(itInv->second, h->InvulnDuration);

            auto itKb = scriptObj.properties.find("knockback");
            if (itKb != scriptObj.properties.end())
                h->KnockbackForce = ParseFloat(itKb->second, 0.0f);
        }

        // -------- Spawner --------
        {
            auto itEvery  = scriptObj.properties.find("spawn_every");
            auto itPrefab = scriptObj.properties.find("spawn_prefab");
            if (itEvery != scriptObj.properties.end() && itPrefab != scriptObj.properties.end()) {
                auto* sp = entity->AddComponent<Spawner>();
                sp->PrefabName = itPrefab->second;
                sp->Interval   = ParseFloat(itEvery->second, 5.0f);
                if (sp->Interval <= 0.0f) sp->Interval = 5.0f;

                auto itDelay = scriptObj.properties.find("spawn_delay");
                if (itDelay != scriptObj.properties.end())
                    sp->InitialDelay = ParseFloat(itDelay->second, 0.0f);

                auto itMax = scriptObj.properties.find("spawn_max");
                if (itMax != scriptObj.properties.end())
                    sp->MaxSpawns = static_cast<int>(ParseFloat(itMax->second, -1.0f));

                auto itOffX = scriptObj.properties.find("spawn_offset_x");
                auto itOffY = scriptObj.properties.find("spawn_offset_y");
                if (itOffX != scriptObj.properties.end())
                    sp->SpawnOffset.x = ParseFloat(itOffX->second, 0.0f);
                if (itOffY != scriptObj.properties.end())
                    sp->SpawnOffset.y = ParseFloat(itOffY->second, 0.0f);

                auto itAtSelf = scriptObj.properties.find("spawn_at_self");
                if (itAtSelf != scriptObj.properties.end())
                    sp->SpawnAtSelf = ParseBool(itAtSelf->second, true);

                RuntimeBridge* self = this;
                Entity* owner = entity;
                sp->SpawnCallback = [self, owner](const std::string& pName, const glm::vec2& pos) {
                    self->SpawnPrefabAt(pName, pos, owner);
                };
            }
        }

        auto itProjSpeed = scriptObj.properties.find("projectile_speed");
        auto itProjLifetime = scriptObj.properties.find("projectile_lifetime");
        auto itProjDamage = scriptObj.properties.find("projectile_damage");
        auto itProjAngle  = scriptObj.properties.find("projectile_angle");
        auto itProjFriendlyTag = scriptObj.properties.find("projectile_friendly_tag");
        if (itProjSpeed != scriptObj.properties.end() ||
            itProjLifetime != scriptObj.properties.end() ||
            itProjDamage != scriptObj.properties.end() ||
            itProjAngle != scriptObj.properties.end() ||
            itProjFriendlyTag != scriptObj.properties.end() ||
            scriptObj.type == "Projectile" ||
            (scriptObj.type == "Prefab" && StartsWith(scriptObj.name, "Bullet")))
        {
            auto* p = entity->AddComponent<Projectile>();
            if (itProjSpeed != scriptObj.properties.end())
                p->Speed = ParseFloat(itProjSpeed->second, p->Speed);
            if (itProjLifetime != scriptObj.properties.end())
                p->Lifetime = ParseFloat(itProjLifetime->second, p->Lifetime);
            if (itProjDamage != scriptObj.properties.end())
                p->Damage = static_cast<int>(ParseFloat(itProjDamage->second, static_cast<float>(p->Damage)));
            if (itProjAngle != scriptObj.properties.end())
                p->AngleDeg = ParseFloat(itProjAngle->second, 0.0f);
            if (itProjFriendlyTag != scriptObj.properties.end())
                p->FriendlyTag = itProjFriendlyTag->second;
        }

        auto itSkipCollision = scriptObj.properties.find("skip_collision");
        bool skipCollision = (itSkipCollision != scriptObj.properties.end() && ParseBool(itSkipCollision->second, false));

        auto itCollider = scriptObj.properties.find("collider");
        bool colliderExplicitOff = (itCollider != scriptObj.properties.end() && itCollider->second != "box");
        bool wantsCollider = !skipCollision && !colliderExplicitOff &&
            ((itCollider != scriptObj.properties.end() && itCollider->second == "box")
            || scriptObj.properties.count("size")
            || scriptObj.properties.count("width")
            || scriptObj.properties.count("height"));

        if (wantsCollider) {
            auto* col = entity->AddComponent<Collider>();
            col->Size = glm::vec2(width, height);

            auto itLayerCol = scriptObj.properties.find("layer");
            if (itLayerCol != scriptObj.properties.end()) col->Layer = static_cast<int>(ParseFloat(itLayerCol->second, 1.0f));

            auto itMask = scriptObj.properties.find("mask");
            if (itMask != scriptObj.properties.end()) col->Mask = static_cast<int>(ParseFloat(itMask->second, -1.0f));

            auto itTrigger = scriptObj.properties.find("trigger");
            if (itTrigger != scriptObj.properties.end()) col->IsTrigger = ParseBool(itTrigger->second, false);

            if (scriptObj.type == "Item" && name.rfind("Coin", 0) == 0)
                col->IsTrigger = true;

            RuntimeBridge* self = this;
            auto pushFn = [self, entity](Entity* otherEnt) {
                if (!otherEnt) return;
                CollisionEventEntry e;
                e.NameA = entity->Name;
                e.TagA  = entity->Tag;
                e.NameB = otherEnt->Name;
                e.TagB  = otherEnt->Tag;
                self->m_PendingCollisions.push_back(e);
                uint64_t key = PairKeyByName(e.NameA, e.NameB);
                self->m_CollisionPairsThis.insert(key);
            };
            col->OnCollision = [pushFn](Entity*, Entity* b, const CollisionManifold&) {
                pushFn(b);
            };
            col->OnTriggerEnter = [pushFn](Entity*, Entity* b) {
                pushFn(b);
            };
        }

        // Determine AI from properties, so prefabs spawned at runtime get AI too.
        auto itBoss = scriptObj.properties.find("boss");
        bool isBoss = (itBoss != scriptObj.properties.end() && ParseBool(itBoss->second, false));

        auto itPatrolMin = scriptObj.properties.find("patrol_min");
        auto itPatrolMax = scriptObj.properties.find("patrol_max");
        bool wantsPatrol = (itPatrolMin != scriptObj.properties.end() && itPatrolMax != scriptObj.properties.end());

        if (isBoss) {
            auto* boss = entity->AddComponent<BossAI>();
            auto itHp = scriptObj.properties.find("hp");
            if (itHp != scriptObj.properties.end()) {
                boss->Health = static_cast<int>(ParseFloat(itHp->second, 3.0f));
                boss->MaxHealth = boss->Health;
            }
            boss->PlayerTarget = playerEntity;
        }
        else if (wantsPatrol || scriptObj.type == "Enemy") {
            auto* patrol = entity->AddComponent<PatrolAI>();
            if (wantsPatrol) {
                patrol->MinX = ParseFloat(itPatrolMin->second, 0.0f);
                patrol->MaxX = ParseFloat(itPatrolMax->second, 0.0f);
            } else {
                auto* t = entity->GetComponent<Transform>();
                float cx = t ? t->Position.x : 0.0f;
                patrol->MinX = cx - 120.0f;
                patrol->MaxX = cx + 120.0f;
            }
            auto itPatrolSpeed = scriptObj.properties.find("patrol_speed");
            if (itPatrolSpeed != scriptObj.properties.end())
                patrol->Speed = ParseFloat(itPatrolSpeed->second, patrol->Speed);
        }

        auto* transform = entity->GetComponent<Transform>();
        if (transform) {
            auto itX = scriptObj.properties.find("x");
            if (itX != scriptObj.properties.end()) transform->Position.x = ParseFloat(itX->second, 0.0f);

            auto itY = scriptObj.properties.find("y");
            if (itY != scriptObj.properties.end()) transform->Position.y = ParseFloat(itY->second, 0.0f);

            auto itRot = scriptObj.properties.find("rotation");
            if (itRot != scriptObj.properties.end()) transform->Rotation = ParseFloat(itRot->second, 0.0f);
        }

        return entity;
    }

    bool RuntimeBridge::BuildObjects() {
        for (const auto& scriptObj : interpreter.GetObjects()) {
            if (scriptObj.type == "Prefab") {
                m_Prefabs[scriptObj.name] = scriptObj;
                std::cout << "[RuntimeBridge] Registered prefab: " << scriptObj.name << "\n";
                continue;
            }
            BuildEntityFromScript(scriptObj.name, scriptObj);
        }

        if (!playerEntity && !entities.empty()) {
            playerEntity = entities.begin()->second;
        }

        for (auto& [name, ent] : entities) {
            if (auto* boss = ent->GetComponent<BossAI>())
                boss->PlayerTarget = playerEntity;
        }

        return true;
    }

    void RuntimeBridge::ProcessTimerEvents(float dt) {
        for (const auto& ev : interpreter.GetEvents()) {
            const std::string& trig = ev.trigger;

            // One-shot "after N" events (fire once, N seconds after level start).
            if (trig.size() >= 6 && trig.substr(0, 6) == "after ") {
                if (m_AfterFired.count(trig)) continue;
                float delay = ParseFloat(trig.substr(6), 1.0f);
                if (delay <= 0.0f) continue;
                float& acc = m_TimerAccum[trig];
                acc += dt;
                if (acc >= delay) {
                    m_AfterFired.insert(trig);
                    ExecuteCommands(ev.commands, playerEntity);
                }
                continue;
            }

            // Repeating "timer N" events.
            if (trig.size() < 6 || trig.substr(0, 6) != "timer ") continue;
            float period = ParseFloat(trig.substr(6), 1.0f);
            if (period <= 0.0f) continue;
            float& acc = m_TimerAccum[trig];
            acc += dt;
            if (acc >= period) {
                acc -= period;
                if (acc >= period) acc = 0.0f;
                ExecuteCommands(ev.commands, playerEntity);
            }
        }
    }

    void RuntimeBridge::ProcessCollisionEvents() {
        if (m_PendingCollisions.empty()) return;

        std::vector<CollisionEventEntry> collisions = std::move(m_PendingCollisions);
        m_PendingCollisions.clear();

        std::unordered_set<uint64_t> firedThisPass;
        struct FireKey {
            uint64_t entityKey;
            uint64_t triggerHash;
            bool operator==(const FireKey& o) const { return entityKey == o.entityKey && triggerHash == o.triggerHash; }
        };
        struct FireKeyHash {
            size_t operator()(const FireKey& k) const {
                return (size_t)(k.entityKey ^ (k.triggerHash * 1099511628211ULL));
            }
        };
        std::unordered_set<FireKey, FireKeyHash> firedEntityEvent;

        auto hashFn = [](const std::string& s) -> uint64_t {
            uint64_t h = 0;
            for (char c : s) h = h * 131 + static_cast<unsigned char>(c);
            return h;
        };

        auto matches = [this](const std::string& name, const std::string& tag, const std::string& target) -> bool {
            if (tag == target) return true;
            if (name == target) return true;
            if (StartsWith(name, target)) return true;
            return false;
        };

        for (const auto& e : collisions) {
            uint64_t pairKey = PairKeyByName(e.NameA, e.NameB);
            bool isNewPair = (m_CollisionPairsLast.count(pairKey) == 0);
            bool alreadyFiredPair = (firedThisPass.count(pairKey) != 0);
            if (!isNewPair || alreadyFiredPair) continue;
            firedThisPass.insert(pairKey);

            {
                Entity* aEnt = GetEntity(e.NameA);
                Entity* bEnt = GetEntity(e.NameB);
                Projectile* aProj = aEnt ? aEnt->GetComponent<Projectile>() : nullptr;
                Projectile* bProj = bEnt ? bEnt->GetComponent<Projectile>() : nullptr;

                auto applyProjectileHit = [&](Projectile* proj, Entity* projEnt, Entity* hitEnt) {
                    if (!proj || proj->HitSomething || !hitEnt) return;
                    if (proj->Shooter == hitEnt->Name) return;
                    if (!proj->FriendlyTag.empty() && proj->FriendlyTag == hitEnt->Tag) return;

                    glm::vec2 src(99999.0f, 99999.0f);
                    if (projEnt) {
                        if (auto* pt = projEnt->GetComponent<Transform>()) src = pt->Position;
                    }

                    if (auto* h = hitEnt->GetComponent<Health>()) {
                        if (h->TakeDamage(proj->Damage, src, 0.6f)) {
                            if (m_Camera) m_Camera->Shake(5.0f, 30.0f, 0.2f);
                            if (h->IsDead()) DestroyEntitySafe(hitEnt);
                        }
                    }
                    proj->HitSomething = true;
                    proj->Lifetime = 0.0f;
                    DestroyEntitySafe(projEnt);
                };

                if (aProj) applyProjectileHit(aProj, aEnt, bEnt);
                if (bProj) applyProjectileHit(bProj, bEnt, aEnt);
            }

            for (const auto& scriptEvent : interpreter.GetEvents()) {
                const std::string& trigger = scriptEvent.trigger;
                if (trigger.size() < 8 || trigger.substr(0, 8) != "collide ") continue;
                std::string target = trigger.substr(8);

                bool matchA = matches(e.NameA, e.TagA, target);
                bool matchB = matches(e.NameB, e.TagB, target);

                Entity* aEntPtr = GetEntity(e.NameA);
                Entity* bEntPtr = GetEntity(e.NameB);
                bool aIsPlayer = (aEntPtr == playerEntity);
                bool bIsPlayer = (bEntPtr == playerEntity);

                // "on collide X" only fires when the OTHER side is the player.
                // Prevents bullet→enemy collisions from running player-damage commands.
                if (matchA && bIsPlayer) {
                    FireKey fk;
                    fk.entityKey = hashFn(e.NameA);
                    fk.triggerHash = hashFn(trigger);
                    if (firedEntityEvent.insert(fk).second) {
                        if (aEntPtr) ExecuteCommands(scriptEvent.commands, aEntPtr);
                    }
                }
                if (matchB && aIsPlayer && (e.NameB != e.NameA)) {
                    FireKey fk;
                    fk.entityKey = hashFn(e.NameB);
                    fk.triggerHash = hashFn(trigger);
                    if (firedEntityEvent.insert(fk).second) {
                        if (bEntPtr) ExecuteCommands(scriptEvent.commands, bEntPtr);
                    }
                }
            }
        }
    }

    Texture* RuntimeBridge::GetOrLoadTexture(const std::string& path) {
        auto it = textures.find(path);
        if (it != textures.end()) return it->second.get();

        auto tex = std::make_unique<Texture>();
        if (!tex->Load(path)) {
            std::cout << "[Texture] Failed: " << path << "\n";
            return nullptr;
        }

        Texture* ptr = tex.get();
        textures[path] = std::move(tex);
        return ptr;
    }

    Sound* RuntimeBridge::GetOrLoadSound(const std::string& name) {
        auto it = sounds.find(name);
        if (it != sounds.end()) return it->second.get();

        auto snd = std::make_unique<Sound>();
        std::string path = SoundPrefix + name + SoundExtension;
        if (!snd->Load(path)) return nullptr;

        Sound* ptr = snd.get();
        sounds[name] = std::move(snd);
        return ptr;
    }

    void RuntimeBridge::ExecuteCommands(const std::vector<std::string>& commands, Entity* contextEntity) {
        m_ContextEntity = contextEntity;
        size_t index = 0;
        while (index < commands.size()) {
            DispatchCommand(commands, index);
        }
    }

    void RuntimeBridge::DispatchCommand(const std::vector<std::string>& commands, size_t& index) {
        std::string token = commands[index];
        ++index;

        Entity* explicitTarget = nullptr;
        size_t dotPos = token.find('.');
        if (dotPos != std::string::npos) {
            std::string targetName = token.substr(0, dotPos);
            token = token.substr(dotPos + 1);
            explicitTarget = GetEntity(targetName);
            if (!explicitTarget) {
                std::cout << "[RuntimeBridge] WARNING: entity '" << targetName
                          << "' not found for command '" << token << "', skipping.\n";
                if (token == "move" || token == "gravity" || token == "play") {
                    if (index < commands.size()) ++index;
                }
                else if (token == "apply_force") {
                    if (index < commands.size()) ++index;
                    if (index < commands.size()) ++index;
                }
                else if (token == "damage" || token == "heal") {
                    if (index < commands.size()) ++index;
                    if (index < commands.size()) ++index;
                }
                else if (token == "look_at") {
                    if (index < commands.size()) ++index;
                    if (index < commands.size()) ++index;
                }
                else if (token == "rotate") {
                    if (index < commands.size()) ++index;
                }
                else if (token == "spawn") {
                    if (index < commands.size()) ++index;
                    if (index < commands.size()) {
                        if (commands[index - 1] == "at") { if (index < commands.size()) ++index; }
                    }
                }
                return;
            }
        }

        Entity* defaultTarget = explicitTarget ? explicitTarget
                             : (m_ContextEntity ? m_ContextEntity : playerEntity);

        if (token == "jump") {
            CmdJump(defaultTarget);
        }
        else if (token == "move") {
            if (index < commands.size()) {
                CmdMove(defaultTarget, commands[index]);
                ++index;
            }
        }
        else if (token == "gravity") {
            if (index < commands.size()) {
                CmdGravity(defaultTarget, ParseBool(commands[index]));
                ++index;
            }
        }
        else if (token == "play") {
            if (index < commands.size()) {
                CmdPlay(commands[index]);
                ++index;
            }
        }
        else if (token == "victory") {
            m_VictoryTriggered = true;
        }
        else if (token == "spawn") {
            std::string prefabName;
            std::string atTarget;
            float x = 0.0f, y = 0.0f;
            int locMode = 0;

            if (index < commands.size()) {
                prefabName = commands[index];
                ++index;
            }

            if (index < commands.size() && commands[index] == "at") {
                ++index;
            }

            if (index < commands.size()) {
                const std::string& loc = commands[index];
                ++index;
                if (LooksLikeNumber(loc)) {
                    x = ParseFloat(loc, 0.0f);
                    if (index < commands.size()) {
                        y = ParseFloat(commands[index], 0.0f);
                        ++index;
                    }
                    locMode = 2;
                }
                else {
                    atTarget = loc;
                    locMode = 1;
                }
            }

            CmdSpawn(explicitTarget, prefabName, atTarget, x, y, locMode);
        }
        else if (token == "despawn") {
            Entity* t = defaultTarget;
            if (index < commands.size()) {
                const std::string& nextTok = commands[index];
                if (!LooksLikeNumber(nextTok) && !nextTok.empty()) {
                    Entity* named = GetEntity(nextTok);
                    if (named) { t = named; ++index; }
                }
            }
            CmdDespawn(t);
        }
        else if (token == "apply_force") {
            float fx = 0.0f, fy = 0.0f;
            if (index < commands.size()) { fx = ParseFloat(commands[index], 0.0f); ++index; }
            if (index < commands.size()) { fy = ParseFloat(commands[index], 0.0f); ++index; }
            CmdApplyForce(defaultTarget, fx, fy);
        }
        else if (token == "damage") {
            std::string entName;
            int amount = 1;
            bool hasEntName = false;

            if (index < commands.size()) {
                const std::string& first = commands[index];
                if (LooksLikeNumber(first)) {
                    amount = static_cast<int>(ParseFloat(first, 1.0f));
                    ++index;
                }
                else {
                    entName = first;
                    hasEntName = true;
                    ++index;
                    if (index < commands.size() && LooksLikeNumber(commands[index])) {
                        amount = static_cast<int>(ParseFloat(commands[index], 1.0f));
                        ++index;
                    }
                }
            }
            CmdDamage(defaultTarget, entName, amount, hasEntName);
        }
        else if (token == "heal") {
            std::string entName;
            int amount = 1;
            bool hasEntName = false;

            if (index < commands.size()) {
                const std::string& first = commands[index];
                if (LooksLikeNumber(first)) {
                    amount = static_cast<int>(ParseFloat(first, 1.0f));
                    ++index;
                }
                else {
                    entName = first;
                    hasEntName = true;
                    ++index;
                    if (index < commands.size() && LooksLikeNumber(commands[index])) {
                        amount = static_cast<int>(ParseFloat(commands[index], 1.0f));
                        ++index;
                    }
                }
            }
            CmdHeal(defaultTarget, entName, amount, hasEntName);
        }
        else if (token == "look_at") {
            Entity* fromTarget = defaultTarget;
            Entity* toTarget = nullptr;
            if (index < commands.size()) {
                Entity* named = GetEntity(commands[index]);
                if (named) { fromTarget = named; ++index; }
            }
            if (index < commands.size()) {
                toTarget = GetEntity(commands[index]);
                ++index;
            }
            if (fromTarget && toTarget) {
                auto* tFrom = fromTarget->GetComponent<Transform>();
                auto* tTo = toTarget->GetComponent<Transform>();
                if (tFrom && tTo) {
                    float dx = tTo->Position.x - tFrom->Position.x;
                    float dy = tTo->Position.y - tFrom->Position.y;
                    tFrom->Rotation = std::atan2(dy, dx) * 180.0f / 3.1415926535f;
                }
            }
        }
        else if (token == "rotate") {
            Entity* rotTarget = defaultTarget;
            float angle = 0.0f;
            if (index < commands.size()) {
                Entity* named = GetEntity(commands[index]);
                if (named) { rotTarget = named; ++index; }
            }
            if (index < commands.size()) {
                angle = ParseFloat(commands[index], 0.0f);
                ++index;
            }
            if (auto* t = rotTarget ? rotTarget->GetComponent<Transform>() : nullptr) {
                t->Rotation = angle;
            }
        }
    }

    void RuntimeBridge::CmdJump(Entity* target) {
        if (!target) return;
        auto* rb = target->GetComponent<RigidBody>();
        if (!rb) return;
        if (!rb->IsOnGround) return;

        float jumpForce = 500.0f;
        const auto* playerObj = interpreter.GetObject(target->Name);
        if (!playerObj) {
            for (const auto& [pname, pobj] : m_Prefabs) {
                if (target->Tag == pobj.name || target->Tag == pobj.type) {
                    playerObj = &pobj;
                    break;
                }
            }
        }
        if (!playerObj && !interpreter.GetObjects().empty()) {
            playerObj = &interpreter.GetObjects()[0];
        }
        if (playerObj) {
            auto it = playerObj->properties.find("jump_force");
            if (it != playerObj->properties.end()) jumpForce = ParseFloat(it->second, 500.0f);
        }

        rb->Velocity.y = -jumpForce;
        rb->IsOnGround = false;
    }

    void RuntimeBridge::CmdMove(Entity* target, const std::string& direction) {
        if (!target) return;
        auto* rb = target->GetComponent<RigidBody>();
        if (!rb) return;

        float speed = 200.0f;
        const auto* playerObj = interpreter.GetObject(target->Name);
        if (!playerObj) {
            for (const auto& [pname, pobj] : m_Prefabs) {
                if (target->Tag == pobj.name || target->Tag == pobj.type) {
                    playerObj = &pobj;
                    break;
                }
            }
        }
        if (!playerObj && !interpreter.GetObjects().empty()) {
            playerObj = &interpreter.GetObjects()[0];
        }
        if (playerObj) {
            auto it = playerObj->properties.find("speed");
            if (it != playerObj->properties.end()) speed = ParseFloat(it->second, 200.0f);
        }

        bool isPlayer = (target == playerEntity);
        if (direction == "left") {
            rb->Velocity.x = -speed;
            if (isPlayer) m_MovedHorizontalThisFrame = true;
        }
        else if (direction == "right") {
            rb->Velocity.x = speed;
            if (isPlayer) m_MovedHorizontalThisFrame = true;
        }
        else if (direction == "up") {
            rb->Velocity.y = -speed;
            if (isPlayer) m_MovedVerticalThisFrame = true;
        }
        else if (direction == "down") {
            rb->Velocity.y = speed;
            if (isPlayer) m_MovedVerticalThisFrame = true;
        }
    }

    void RuntimeBridge::CmdGravity(Entity* target, bool enabled) {
        if (!target) return;
        auto* rb = target->GetComponent<RigidBody>();
        if (!rb) return;
        rb->UseGravity = enabled;
    }

    void RuntimeBridge::CmdPlay(const std::string& name) {
        Sound* snd = GetOrLoadSound(name);
        if (snd) snd->Play();
    }

    void RuntimeBridge::CmdSpawn(Entity* explicitTarget, const std::string& prefabName,
                                 const std::string& atTarget, float x, float y, int locMode) {
        auto itPrefab = m_Prefabs.find(prefabName);
        if (itPrefab == m_Prefabs.end()) {
            std::cout << "[RuntimeBridge] Spawn: prefab '" << prefabName << "' not found\n";
            return;
        }

        ++m_SpawnCounter;
        std::string newName = prefabName + "_" + std::to_string(m_SpawnCounter);

        const ScriptObject& prefab = itPrefab->second;
        Entity* newEntity = BuildEntityFromScript(newName, prefab);
        if (!newEntity) return;

        auto* t = newEntity->GetComponent<Transform>();
        if (!t) return;

        Entity* shooter = explicitTarget ? explicitTarget
                       : (m_ContextEntity ? m_ContextEntity : playerEntity);

        if (auto* proj = newEntity->GetComponent<Projectile>()) {
            if (shooter) {
                proj->Shooter = shooter->Name;
                if (!shooter->Tag.empty()) proj->FriendlyTag = shooter->Tag;

                if (proj->AngleDeg == 0.0f) {
                    if (auto* st = shooter->GetComponent<Transform>()) {
                        t->Rotation = st->Rotation;
                    }
                }
            }
        }

        glm::vec2 finalPos(0.0f, 0.0f);
        bool havePos = false;

        if (locMode == 1) {
            Entity* loc = GetEntity(atTarget);
            if (!loc) loc = m_ContextEntity;
            if (!loc) loc = playerEntity;
            if (loc) {
                if (auto* tl = loc->GetComponent<Transform>()) {
                    finalPos = tl->Position;
                    havePos = true;
                }
            }
        }
        else if (locMode == 2) {
            finalPos = { x, y };
            havePos = true;
        }
        else {
            Entity* loc = explicitTarget;
            if (!loc) loc = m_ContextEntity;
            if (!loc) loc = playerEntity;
            if (loc) {
                if (auto* tl = loc->GetComponent<Transform>()) {
                    finalPos = tl->Position;
                    havePos = true;
                    if (auto* proj = newEntity->GetComponent<Projectile>()) {
                        if (proj->AngleDeg == 0.0f) {
                            t->Rotation = tl->Rotation;
                        }
                    }
                }
            }
        }

        if (!havePos) {
            std::cout << "[RuntimeBridge] WARNING: spawn '" << prefabName
                      << "' has no position, placing at origin.\n";
        }
        t->Position = finalPos;
    }

    Entity* RuntimeBridge::SpawnPrefabAt(const std::string& prefabName,
                                         const glm::vec2& position, Entity* shooter) {
        auto itPrefab = m_Prefabs.find(prefabName);
        if (itPrefab == m_Prefabs.end()) {
            std::cout << "[RuntimeBridge] SpawnPrefabAt: prefab '" << prefabName
                      << "' not found\n";
            return nullptr;
        }

        ++m_SpawnCounter;
        std::string newName = prefabName + "_" + std::to_string(m_SpawnCounter);
        Entity* newEntity = BuildEntityFromScript(newName, itPrefab->second);

        if (auto* t = newEntity ? newEntity->GetComponent<Transform>() : nullptr) {
            t->Position = position;
        }

        if (auto* proj = newEntity ? newEntity->GetComponent<Projectile>() : nullptr) {
            if (shooter) {
                proj->Shooter = shooter->Name;
                if (!shooter->Tag.empty()) proj->FriendlyTag = shooter->Tag;
                if (proj->AngleDeg == 0.0f) {
                    if (auto* st = shooter->GetComponent<Transform>()) {
                        if (auto* nt = newEntity->GetComponent<Transform>())
                            nt->Rotation = st->Rotation;
                    }
                }
            }
        }
        return newEntity;
    }

    void RuntimeBridge::CmdDespawn(Entity* target) {
        if (!target) return;
        DestroyEntitySafe(target);
    }

    void RuntimeBridge::CmdApplyForce(Entity* target, float fx, float fy) {
        if (!target) return;
        auto* rb = target->GetComponent<RigidBody>();
        if (!rb) return;
        rb->ApplyImpulse({ fx, fy });
    }

    void RuntimeBridge::CmdDamage(Entity* defaultTarget, const std::string& entName,
                                  int amount, bool hasEntName) {
        Entity* t = defaultTarget;
        if (hasEntName && !entName.empty()) {
            Entity* named = GetEntity(entName);
            if (named) t = named;
            else {
                std::cout << "[RuntimeBridge] WARNING: damage target '" << entName
                          << "' not found, skipping.\n";
                return;
            }
        }
        if (!t) return;
        if (amount <= 0) amount = 1;

        glm::vec2 sourcePos(0.0f, 0.0f);
        bool haveSource = false;
        if (m_ContextEntity && m_ContextEntity != t) {
            if (auto* ct = m_ContextEntity->GetComponent<Transform>()) {
                sourcePos = ct->Position;
                haveSource = true;
            }
        }
        if (!haveSource) sourcePos = glm::vec2(99999.0f, 99999.0f);

        if (auto* h = t->GetComponent<Health>()) {
            if (h->TakeDamage(amount, sourcePos)) {
                if (m_Camera) m_Camera->Shake(6.0f, 30.0f, 0.25f);
                if (h->IsDead()) DestroyEntitySafe(t);
            }
            return;
        }

        if (auto* boss = t->GetComponent<BossAI>()) {
            for (int i = 0; i < amount; ++i)
                if (!boss->TakeStompDamage()) break;
            if (m_Camera) m_Camera->Shake(4.0f, 30.0f, 0.2f);
            if (boss->IsDefeated()) DestroyEntitySafe(t);
        }
    }

    void RuntimeBridge::CmdHeal(Entity* defaultTarget, const std::string& entName,
                                int amount, bool hasEntName) {
        Entity* t = defaultTarget;
        if (hasEntName && !entName.empty()) {
            Entity* named = GetEntity(entName);
            if (named) t = named;
            else {
                std::cout << "[RuntimeBridge] WARNING: heal target '" << entName
                          << "' not found, skipping.\n";
                return;
            }
        }
        if (!t) return;
        if (amount <= 0) return;

        if (auto* h = t->GetComponent<Health>()) {
            h->Current += amount;
            if (h->Current > h->Max) h->Current = h->Max;
            return;
        }
        if (auto* boss = t->GetComponent<BossAI>()) {
            boss->Health += amount;
            if (boss->Health > boss->MaxHealth) boss->Health = boss->MaxHealth;
        }
    }

    float RuntimeBridge::ParseFloat(const std::string& str, float defaultVal) {
        try {
            return std::stof(str);
        }
        catch (...) {
            return defaultVal;
        }
    }

    bool RuntimeBridge::ParseBool(const std::string& str, bool defaultVal) {
        if (str == "true" || str == "on" || str == "yes" || str == "1") return true;
        if (str == "false" || str == "off" || str == "no" || str == "0") return false;
        return defaultVal;
    }

    void RuntimeBridge::NullifyPlayerReferences() {
        for (auto& [name, ent] : entities) {
            if (!ent) continue;
            if (auto* boss = ent->GetComponent<BossAI>()) {
                if (boss->PlayerTarget == playerEntity) boss->PlayerTarget = nullptr;
            }
        }
        playerEntity = nullptr;
        m_ContextEntity = nullptr;
    }

    void RuntimeBridge::DestroyEntitySafe(Entity* e) {
        if (!e) return;
        bool wasPlayer = (e == playerEntity);
        std::string nm = e->Name;
        scene->DestroyEntity(e);
        entities.erase(nm);
        if (wasPlayer) {
            m_PlayerDead = true;
            NullifyPlayerReferences();
        }
    }

    int RuntimeBridge::StringToScancode(const std::string& key) {
        if (key == "a" || key == "A") return SDL_SCANCODE_A;
        if (key == "b" || key == "B") return SDL_SCANCODE_B;
        if (key == "c" || key == "C") return SDL_SCANCODE_C;
        if (key == "d" || key == "D") return SDL_SCANCODE_D;
        if (key == "e" || key == "E") return SDL_SCANCODE_E;
        if (key == "f" || key == "F") return SDL_SCANCODE_F;
        if (key == "g" || key == "G") return SDL_SCANCODE_G;
        if (key == "h" || key == "H") return SDL_SCANCODE_H;
        if (key == "i" || key == "I") return SDL_SCANCODE_I;
        if (key == "j" || key == "J") return SDL_SCANCODE_J;
        if (key == "k" || key == "K") return SDL_SCANCODE_K;
        if (key == "l" || key == "L") return SDL_SCANCODE_L;
        if (key == "m" || key == "M") return SDL_SCANCODE_M;
        if (key == "n" || key == "N") return SDL_SCANCODE_N;
        if (key == "o" || key == "O") return SDL_SCANCODE_O;
        if (key == "p" || key == "P") return SDL_SCANCODE_P;
        if (key == "q" || key == "Q") return SDL_SCANCODE_Q;
        if (key == "r" || key == "R") return SDL_SCANCODE_R;
        if (key == "s" || key == "S") return SDL_SCANCODE_S;
        if (key == "t" || key == "T") return SDL_SCANCODE_T;
        if (key == "u" || key == "U") return SDL_SCANCODE_U;
        if (key == "v" || key == "V") return SDL_SCANCODE_V;
        if (key == "w" || key == "W") return SDL_SCANCODE_W;
        if (key == "x" || key == "X") return SDL_SCANCODE_X;
        if (key == "y" || key == "Y") return SDL_SCANCODE_Y;
        if (key == "z" || key == "Z") return SDL_SCANCODE_Z;
        if (key == "space" || key == "Space") return SDL_SCANCODE_SPACE;
        if (key == "enter" || key == "Enter") return SDL_SCANCODE_RETURN;
        if (key == "escape" || key == "Escape") return SDL_SCANCODE_ESCAPE;
        if (key == "up" || key == "Up") return SDL_SCANCODE_UP;
        if (key == "down" || key == "Down") return SDL_SCANCODE_DOWN;
        if (key == "left" || key == "Left") return SDL_SCANCODE_LEFT;
        if (key == "right" || key == "Right") return SDL_SCANCODE_RIGHT;
        return -1;
    }

    void Projectile::Update(float dt) {
        if (!m_Owner) return;
        if (HitSomething) return;

        Lifetime -= dt;

        auto* t = m_Owner->GetComponent<Transform>();
        if (!t) return;

        float angleDeg = AngleDeg != 0.0f ? AngleDeg : t->Rotation;
        float rad = angleDeg * 3.1415926535f / 180.0f;
        float dx = std::cos(rad) * Speed * dt;
        float dy = std::sin(rad) * Speed * dt;

        if (auto* rb = m_Owner->GetComponent<RigidBody>()) {
            rb->Velocity.x = dx / std::max(0.0001f, dt);
            rb->Velocity.y = dy / std::max(0.0001f, dt);
        } else {
            t->Position.x += dx;
            t->Position.y += dy;
        }
    }

} // namespace ar