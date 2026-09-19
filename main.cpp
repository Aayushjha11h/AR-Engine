#include "engine/GameLoop.h"
#include "engine/Input.h"
#include "engine/Scene.h"
#include "engine/Audio.h"
#include "engine/Camera.h"
#include "Language/RuntimeBridge.h"

#include <fstream>
#include <iostream>

#undef main

class ScriptGame : public ar::GameLoop {
public:
    ar::RuntimeBridge* bridge = nullptr;

    void OnInit() override {

        bridge = new ar::RuntimeBridge(
            GetScene(),
            GetInput(),
            GetAudio()
        );

        std::ifstream file("game.argdl");

        if (!file) {
            std::cerr << "game.argdl not found.\n";
            return;
        }

        std::string script(
            (std::istreambuf_iterator<char>(file)),
            std::istreambuf_iterator<char>()
        );

        if (!bridge->Load(script)) {
            std::cerr << bridge->GetError() << '\n';
            return;
        }

        std::cout << "[Game] Script loaded successfully.\n";

        if (GetScene()->FindEntity("Player"))
            std::cout << "[Game] Player exists.\n";

        if (GetScene()->FindEntity("Ground"))
            std::cout << "[Game] Ground exists.\n";

        if (GetScene()->FindEntity("Platform1"))
            std::cout << "[Game] Platform1 exists.\n";

        if (GetScene()->FindEntity("Platform2"))
            std::cout << "[Game] Platform2 exists.\n";

        if (GetScene()->FindEntity("Platform3"))
            std::cout << "[Game] Platform3 exists.\n";
    }

    void OnUpdate(float dt) override {

        bridge->Update(dt);

        auto* player = GetScene()->FindEntity("Player");

        if (player) {

            auto* t = player->GetComponent<ar::Transform>();

            if (t)
                GetCamera()->Follow(t->Position, 5.0f, dt);
        }

        float scroll = GetInput()->GetMouseScroll();

        if (scroll != 0)
            GetCamera()->Zoom(-scroll * 0.1f);
    }

    void OnShutdown() override {
        delete bridge;
    }
};

int main() {

    ScriptGame game;

    if (!game.Init("AR Script Demo", 800, 600))
        return -1;

    game.Run();

    return 0;
}