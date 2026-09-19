#include "RuntimeBridge.h"
#include <SDL2/SDL.h>
#include <iostream>

namespace ar {

    RuntimeBridge::RuntimeBridge(Scene* scene, Input* input, Audio* audio)
        : scene(scene), input(input), audio(audio) {
    }

    bool RuntimeBridge::Load(const std::string& argdlSource) {
        entities.clear();
        playerEntity = nullptr;
        textures.clear();
        sounds.clear();
        error.clear();

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
        for (const auto& scriptEvent : interpreter.GetEvents()) {
            int scancode = StringToScancode(scriptEvent.trigger);
            if (scancode < 0) continue;

            bool isJumpEvent = false;
            for (const auto& cmd : scriptEvent.commands) {
                if (cmd == "jump") { isJumpEvent = true; break; }
            }

            bool shouldFire = isJumpEvent ? input->IsKeyPressed(scancode)
                : input->IsKeyDown(scancode);

            if (shouldFire) {
                ExecuteCommands(scriptEvent.commands);
            }
        }

        if (playerEntity) {
            auto* rb = playerEntity->GetComponent<RigidBody>();
            if (rb) {
                bool leftHeld = input->IsKeyDown(SDL_SCANCODE_A);
                bool rightHeld = input->IsKeyDown(SDL_SCANCODE_D);
                if (!leftHeld && !rightHeld) {
                    rb->Velocity.x = 0.0f;
                }
            }
        }
    }

    Entity* RuntimeBridge::GetEntity(const std::string& name) const {
        auto it = entities.find(name);
        return (it != entities.end()) ? it->second : nullptr;
    }

    // -------------------------------------------------------------------
    // Helpers (free functions, not class members)
    // -------------------------------------------------------------------

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

    // -------------------------------------------------------------------
    // Object Building
    // -------------------------------------------------------------------

    bool RuntimeBridge::BuildObjects() {
        for (const auto& scriptObj : interpreter.GetObjects()) {
            Entity* entity = scene->CreateEntity(scriptObj.name, scriptObj.type);
            entities[scriptObj.name] = entity;

            if (!playerEntity && (scriptObj.name == "Player" || scriptObj.type == "Character")) {
                playerEntity = entity;
                std::cout << "[RuntimeBridge] Set as player entity\n";
            }

            std::cout << "[RuntimeBridge] Creating entity: " << scriptObj.name
                << " of type " << scriptObj.type << "\n";

            entity->AddComponent<Transform>();

            auto* sr = entity->AddComponent<SpriteRenderer>(nullptr);
            {
                auto itColor = scriptObj.properties.find("color");
                if (itColor != scriptObj.properties.end()) {
                    sr->SetColor(ParseColorName(itColor->second));
                }
                else {
                    sr->SetColor(TypeColor(scriptObj.type));
                }

                auto itSize = scriptObj.properties.find("size");
                float size = (itSize != scriptObj.properties.end())
                    ? ParseFloat(itSize->second, 64.0f) : 64.0f;
                sr->SetSize(glm::vec2(size, size));
            }

            auto itSprite = scriptObj.properties.find("sprite");
            if (itSprite != scriptObj.properties.end()) {
                Texture* tex = GetOrLoadTexture(itSprite->second);
                if (tex) sr->SetTexture(tex);
            }

            auto itGravity = scriptObj.properties.find("gravity");
            bool wantsGravity = (itGravity != scriptObj.properties.end() && ParseBool(itGravity->second));

            bool wantsPhysics = wantsGravity ||
                scriptObj.type == "Character" ||
                scriptObj.type == "Enemy" ||
                scriptObj.type == "Item" ||
                scriptObj.properties.count("mass") ||
                scriptObj.properties.count("kinematic");

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

            auto itCollider = scriptObj.properties.find("collider");
            bool wantsCollider = (itCollider != scriptObj.properties.end() && itCollider->second == "box")
                || scriptObj.properties.count("size");

            if (wantsCollider) {
                auto* col = entity->AddComponent<Collider>();

                auto itSize = scriptObj.properties.find("size");
                if (itSize != scriptObj.properties.end()) {
                    float s = ParseFloat(itSize->second, 1.0f);
                    col->Size = glm::vec2(s, s);
                }

                auto itLayer = scriptObj.properties.find("layer");
                if (itLayer != scriptObj.properties.end()) col->Layer = static_cast<int>(ParseFloat(itLayer->second, 1.0f));

                auto itMask = scriptObj.properties.find("mask");
                if (itMask != scriptObj.properties.end()) col->Mask = static_cast<int>(ParseFloat(itMask->second, -1.0f));

                auto itTrigger = scriptObj.properties.find("trigger");
                if (itTrigger != scriptObj.properties.end()) col->IsTrigger = ParseBool(itTrigger->second, false);
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
        }

        if (!playerEntity && !entities.empty()) {
            playerEntity = entities.begin()->second;
        }

        return true;
    }

    // -------------------------------------------------------------------
    // Assets
    // -------------------------------------------------------------------

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

    // -------------------------------------------------------------------
    // Commands
    // -------------------------------------------------------------------

    void RuntimeBridge::ExecuteCommands(const std::vector<std::string>& commands) {
        size_t index = 0;
        while (index < commands.size()) {
            DispatchCommand(commands, index);
        }
    }

    void RuntimeBridge::DispatchCommand(const std::vector<std::string>& commands, size_t& index) {
        const std::string& cmd = commands[index];
        ++index;

        if (cmd == "jump") {
            CmdJump();
        }
        else if (cmd == "move") {
            if (index < commands.size()) {
                CmdMove(commands[index]);
                ++index;
            }
        }
        else if (cmd == "gravity") {
            if (index < commands.size()) {
                CmdGravity(ParseBool(commands[index]));
                ++index;
            }
        }
        else if (cmd == "play") {
            if (index < commands.size()) {
                CmdPlay(commands[index]);
                ++index;
            }
        }
    }

    void RuntimeBridge::CmdJump() {
        if (!playerEntity) return;
        auto* rb = playerEntity->GetComponent<RigidBody>();
        if (!rb) return;
        if (!rb->IsOnGround) return;

        float jumpForce = 500.0f;
        const auto* playerObj = interpreter.GetObject("Player");
        if (!playerObj && !interpreter.GetObjects().empty()) {
            playerObj = &interpreter.GetObjects()[0];
        }
        if (playerObj) {
            auto it = playerObj->properties.find("jump_force");
            if (it != playerObj->properties.end()) jumpForce = ParseFloat(it->second, 500.0f);
        }

        rb->ApplyImpulse(glm::vec2(0.0f, jumpForce));
    }

    void RuntimeBridge::CmdMove(const std::string& direction) {
        if (!playerEntity) return;
        auto* rb = playerEntity->GetComponent<RigidBody>();
        if (!rb) return;

        float speed = 200.0f;
        const auto* playerObj = interpreter.GetObject("Player");
        if (!playerObj && !interpreter.GetObjects().empty()) {
            playerObj = &interpreter.GetObjects()[0];
        }
        if (playerObj) {
            auto it = playerObj->properties.find("speed");
            if (it != playerObj->properties.end()) speed = ParseFloat(it->second, 200.0f);
        }

        if (direction == "left")       rb->Velocity.x = -speed;
        else if (direction == "right") rb->Velocity.x = speed;
        else if (direction == "up")    rb->Velocity.y = -speed;
        else if (direction == "down")  rb->Velocity.y = speed;
    }

    void RuntimeBridge::CmdGravity(bool enabled) {
        if (!playerEntity) return;
        auto* rb = playerEntity->GetComponent<RigidBody>();
        if (!rb) return;
        rb->UseGravity = enabled;
    }

    void RuntimeBridge::CmdPlay(const std::string& name) {
        Sound* snd = GetOrLoadSound(name);
        if (snd) snd->Play();
    }

    // -------------------------------------------------------------------
    // Utilities
    // -------------------------------------------------------------------

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

} // namespace ar