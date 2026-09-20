#pragma once
#include "Interpreter.h"
#include "../engine/Scene.h"
#include "../engine/Entity.h"
#include "../engine/Input.h"
#include "../engine/Audio.h"
#include "../engine/Sound.h"
#include "../engine/Texture.h"
#include "../engine/Component.h"
#include "../engine/RigidBody.h"
#include "../engine/Collision.h"
#include "../engine/Health.h"
#include "../engine/Spawner.h"
#include "../engine/Camera.h"
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <vector>
#include <utility>
#include <cstdint>

namespace ar {

    class Projectile : public Component {
    public:
        float Speed        = 400.0f;
        float Lifetime     = 3.0f;
        int   Damage       = 1;
        std::string Shooter;
        std::string FriendlyTag;
        float AngleDeg     = 0.0f;
        bool  HitSomething = false;

        void Update(float dt) override;
    };

    struct CollisionEventEntry {
        std::string NameA;
        std::string TagA;
        std::string NameB;
        std::string TagB;
    };

    class RuntimeBridge {
    public:
        RuntimeBridge(Scene* scene, Input* input, Audio* audio, Camera* camera = nullptr);
        bool Load(const std::string& argdlSource);
        void Update(float dt);
        Entity* GetEntity(const std::string& name) const;
        Entity* GetPlayer() const { return playerEntity; }
        const std::string& GetError() const { return error; }
        int  GetCoinsCollected() const { return m_CoinsCollected; }
        bool IsVictoryTriggered() const { return m_VictoryTriggered; }
        bool IsPlayerDead() const { return m_PlayerDead; }
        void ClearGameplayFlags() { m_VictoryTriggered = false; m_PlayerDead = false; }
        Texture* GetBackgroundTexture() const { return m_BackgroundTexture; }

        // Public spawn helper used by Spawner callbacks.
        Entity* SpawnPrefabAt(const std::string& prefabName, const glm::vec2& position,
                              Entity* shooter = nullptr);

        std::string SoundPrefix = "sounds/";
        std::string SoundExtension = ".wav";

    private:
        Scene*  scene;
        Input*  input;
        Audio*  audio;
        Camera* m_Camera = nullptr;

        Interpreter interpreter;
        std::unordered_map<std::string, Entity*> entities;
        Entity* playerEntity = nullptr;
        std::string error;
        std::unordered_map<std::string, std::unique_ptr<Texture>> textures;
        std::unordered_map<std::string, std::unique_ptr<Sound>>   sounds;

        bool m_MovedHorizontalThisFrame = false;
        bool m_MovedVerticalThisFrame   = false;
        int  m_CoinsCollected   = 0;
        bool m_VictoryTriggered = false;
        bool m_PlayerDead       = false;
        Texture* m_BackgroundTexture = nullptr;

        std::unordered_map<std::string, ScriptObject> m_Prefabs;
        std::vector<CollisionEventEntry>              m_PendingCollisions;
        Entity* m_ContextEntity = nullptr;
        int     m_SpawnCounter  = 0;

        std::unordered_set<uint64_t> m_CollisionPairsLast;
        std::unordered_set<uint64_t> m_CollisionPairsThis;

        std::unordered_map<std::string, float> m_TimerAccum;

        static uint64_t PairKeyByName(const std::string& a, const std::string& b);

        Texture* GetOrLoadTexture(const std::string& path);
        Sound*   GetOrLoadSound(const std::string& name);
        bool     BuildObjects();
        Entity*  BuildEntityFromScript(const std::string& name, const ScriptObject& scriptObj);

        void ExecuteCommands(const std::vector<std::string>& commands, Entity* contextEntity = nullptr);
        void DispatchCommand(const std::vector<std::string>& commands, size_t& index);
        void CmdJump(Entity* target);
        void CmdMove(Entity* target, const std::string& direction);
        void CmdGravity(Entity* target, bool enabled);
        void CmdPlay(const std::string& name);
        void CmdSpawn(Entity* explicitTarget, const std::string& prefabName,
                      const std::string& atTarget, float x, float y, int locMode);
        void CmdDespawn(Entity* target);
        void CmdApplyForce(Entity* target, float fx, float fy);
        void CmdDamage(Entity* defaultTarget, const std::string& entName, int amount, bool hasEntName);
        void CmdHeal(Entity* defaultTarget, const std::string& entName, int amount, bool hasEntName);

        void ProcessCollisionEvents();
        void ProcessTimerEvents(float dt);
        void DestroyEntitySafe(Entity* e);
        void NullifyPlayerReferences();
        Entity* ResolveTarget(const std::string& name);
        static bool StartsWith(const std::string& s, const std::string& prefix);
        static bool LooksLikeNumber(const std::string& s);
        static int  StringToScancode(const std::string& key);
        static float ParseFloat(const std::string& str, float defaultVal);
        static bool  ParseBool(const std::string& str, bool defaultVal = false);
    };

} // namespace ar
