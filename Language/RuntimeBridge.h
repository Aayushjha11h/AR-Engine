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
#include <string>
#include <unordered_map>
#include <memory>

namespace ar {

    class RuntimeBridge {
    public:
        RuntimeBridge(Scene* scene, Input* input, Audio* audio);
        bool Load(const std::string& argdlSource);
        void Update(float dt);
        Entity* GetEntity(const std::string& name) const;
        Entity* GetPlayer() const { return playerEntity; }
        const std::string& GetError() const { return error; }
        std::string SoundPrefix = "sounds/";
        std::string SoundExtension = ".wav";

    private:
        Scene* scene;
        Input* input;
        Audio* audio;
        Interpreter interpreter;
        std::unordered_map<std::string, Entity*> entities;
        Entity* playerEntity = nullptr;
        std::string error;
        std::unordered_map<std::string, std::unique_ptr<Texture>> textures;
        std::unordered_map<std::string, std::unique_ptr<Sound>> sounds;

        Texture* GetOrLoadTexture(const std::string& path);
        Sound* GetOrLoadSound(const std::string& name);
        bool BuildObjects();
        void ExecuteCommands(const std::vector<std::string>& commands);
        void DispatchCommand(const std::vector<std::string>& commands, size_t& index);
        void CmdJump();
        void CmdMove(const std::string& direction);
        void CmdGravity(bool enabled);
        void CmdPlay(const std::string& name);
        static int StringToScancode(const std::string& key);
        static float ParseFloat(const std::string& str, float defaultVal);
        static bool ParseBool(const std::string& str, bool defaultVal = false);
    };

} // namespace ar