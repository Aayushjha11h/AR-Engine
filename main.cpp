#include "engine/GameLoop.h"
#include "engine/Input.h"
#include "engine/Scene.h"
#include "engine/Audio.h"
#include "engine/Camera.h"
#include "engine/Renderer.h"
#include "engine/Texture.h"
#include "engine/Sprite.h"
#include "Language/RuntimeBridge.h"

#include <glm/glm.hpp>
#include <SDL2/SDL.h>
#include <fstream>
#include <iostream>
#include <string>

#undef main

class ScriptGame : public ar::GameLoop {
public:
    ar::RuntimeBridge* bridge = nullptr;
    std::string scriptSource;
    ar::Texture titleTexture;

    void OnInit() override {
        bridge = new ar::RuntimeBridge(GetScene(), GetInput(), GetAudio());

        std::ifstream file("game.argdl");
        if (!file) {
            std::cerr << "game.argdl not found.\n";
            return;
        }

        scriptSource.assign(
            (std::istreambuf_iterator<char>(file)),
            std::istreambuf_iterator<char>());

        titleTexture.Load("assets/title.png");

        SetState(ar::GameState::TitleScreen);
        std::cout << "[Game] Press Enter or Space on title screen to start.\n";
    }

    bool ReloadLevel() {
        if (!bridge || scriptSource.empty()) return false;
        GetScene()->Clear();
        if (!bridge->Load(scriptSource)) {
            std::cerr << bridge->GetError() << '\n';
            return false;
        }
        bridge->ClearGameplayFlags();
        return true;
    }

    void OnUpdate(float dt) override {
        auto* input = GetInput();
        const ar::GameState state = GetState();

        if (state == ar::GameState::TitleScreen) {
            if (input->IsKeyPressed(SDL_SCANCODE_RETURN) || input->IsKeyPressed(SDL_SCANCODE_SPACE)) {
                if (ReloadLevel())
                    SetState(ar::GameState::Playing);
            }
            return;
        }

        if (state == ar::GameState::GameOver) {
            if (input->IsKeyPressed(SDL_SCANCODE_R)) {
                if (ReloadLevel())
                    SetState(ar::GameState::Playing);
            }
            return;
        }

        if (state == ar::GameState::Victory) {
            if (input->IsKeyPressed(SDL_SCANCODE_SPACE) || input->IsKeyPressed(SDL_SCANCODE_RETURN)) {
                if (ReloadLevel())
                    SetState(ar::GameState::Playing);
            }
            return;
        }

        bridge->Update(dt);

        auto* player = bridge->GetPlayer();
        if (player) {
            auto* t = player->GetComponent<ar::Transform>();
            if (t) {
                GetCamera()->Follow(t->Position, 5.0f, dt);
                if (t->Position.y > 620.0f)
                    SetState(ar::GameState::GameOver);
            }
        }

        if (bridge->IsPlayerDead())
            SetState(ar::GameState::GameOver);
        if (bridge->IsVictoryTriggered())
            SetState(ar::GameState::Victory);

        float scroll = input->GetMouseScroll();
        if (scroll != 0.0f)
            GetCamera()->Zoom(-scroll * 0.1f);
    }

    void OnPreRender() override {
        auto* cam = GetCamera();
        const float w = cam->GetZoom() > 0.0f ? 800.0f / cam->GetZoom() : 800.0f;
        const float h = cam->GetZoom() > 0.0f ? 600.0f / cam->GetZoom() : 600.0f;

        GetRenderer()->DrawParallaxBackground(
            cam->GetPosition(), w, h,
            bridge ? bridge->GetBackgroundTexture() : nullptr,
            0.2f,
            { 0.35f, 0.55f, 0.95f, 1.0f });
    }

    void DrawBanner(const glm::vec2& center, const glm::vec2& size, const glm::vec4& color, const glm::vec4& accent) {
        auto* r = GetRenderer();
        r->DrawQuad(center, size, color);
        r->DrawQuad({ center.x, center.y + size.y * 0.35f }, { size.x * 0.85f, size.y * 0.12f }, accent);
    }

    void OnRender() override {
        auto* cam = GetCamera();
        const glm::vec2 camPos = cam->GetPosition();
        const float w = 800.0f;
        const float h = 600.0f;
        auto* r = GetRenderer();

        const ar::GameState state = GetState();

        if (state == ar::GameState::TitleScreen) {
            if (titleTexture.GetID()) {
                ar::Sprite s;
                s.Position = camPos;
                s.Size = { w * 0.9f, h * 0.55f };
                s.Tex = &titleTexture;
                r->DrawSprite(s);
            }
            else {
                DrawBanner(camPos, { w * 0.85f, h * 0.45f }, { 0.1f, 0.12f, 0.25f, 0.95f }, { 0.9f, 0.25f, 0.2f, 1.0f });
            }
            r->DrawQuad({ camPos.x, camPos.y - h * 0.22f }, { w * 0.55f, 36.0f }, { 1.0f, 1.0f, 1.0f, 0.9f });
            r->DrawQuad({ camPos.x, camPos.y + h * 0.28f }, { w * 0.65f, 24.0f }, { 1.0f, 0.95f, 0.4f, 0.85f });
            return;
        }

        if (state == ar::GameState::Playing && bridge) {
            r->DrawQuad({ camPos.x - w * 0.42f, camPos.y - h * 0.42f }, { 140.0f, 36.0f }, { 0.0f, 0.0f, 0.0f, 0.45f });
            r->DrawQuad({ camPos.x - w * 0.42f, camPos.y - h * 0.42f }, { 24.0f + bridge->GetCoinsCollected() * 8.0f, 20.0f }, { 1.0f, 0.85f, 0.1f, 0.95f });
        }

        if (state == ar::GameState::GameOver) {
            r->DrawScreenOverlay(camPos, w, h, { 0.0f, 0.0f, 0.0f, 0.55f });
            DrawBanner(camPos, { w * 0.7f, h * 0.25f }, { 0.35f, 0.05f, 0.05f, 0.95f }, { 0.9f, 0.15f, 0.15f, 1.0f });
            r->DrawQuad({ camPos.x, camPos.y + h * 0.18f }, { w * 0.5f, 20.0f }, { 1.0f, 1.0f, 1.0f, 0.75f });
        }

        if (state == ar::GameState::Victory) {
            r->DrawScreenOverlay(camPos, w, h, { 0.0f, 0.05f, 0.1f, 0.5f });
            DrawBanner(camPos, { w * 0.75f, h * 0.28f }, { 0.05f, 0.35f, 0.15f, 0.95f }, { 0.2f, 0.95f, 0.35f, 1.0f });
            const int coins = bridge ? bridge->GetCoinsCollected() : 0;
            r->DrawQuad({ camPos.x, camPos.y + h * 0.05f }, { 80.0f + coins * 6.0f, 22.0f }, { 1.0f, 0.9f, 0.2f, 1.0f });
            r->DrawQuad({ camPos.x, camPos.y + h * 0.2f }, { w * 0.55f, 18.0f }, { 1.0f, 1.0f, 1.0f, 0.8f });
        }
    }

    void OnShutdown() override {
        delete bridge;
        bridge = nullptr;
    }
};

int main() {
    ScriptGame game;

    if (!game.Init("AR Script Demo", 800, 600))
        return -1;

    game.Run();
    return 0;
}
