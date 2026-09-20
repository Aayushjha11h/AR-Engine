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
#include <cmath>

#undef main

class ScriptGame : public ar::GameLoop {
public:
    ar::RuntimeBridge* bridge = nullptr;
    std::string scriptSource;
    ar::Texture titleTexture;
    float m_Elapsed = 0.0f;

    void OnInit() override {
        bridge = new ar::RuntimeBridge(GetScene(), GetInput(), GetAudio(), GetCamera());

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
        m_Elapsed = 0.0f;
        return true;
    }

    void OnUpdate(float dt) override {
        m_Elapsed += dt;

        auto* input = GetInput();
        const ar::GameState state = GetState();

        if (state == ar::GameState::TitleScreen) {
            if (input->IsKeyPressed(SDL_SCANCODE_RETURN) || input->IsKeyPressed(SDL_SCANCODE_SPACE)) {
                if (ReloadLevel()) SetState(ar::GameState::Playing);
            }
            return;
        }

        if (state == ar::GameState::GameOver) {
            if (input->IsKeyPressed(SDL_SCANCODE_R)) {
                if (ReloadLevel()) SetState(ar::GameState::Playing);
            }
            return;
        }

        if (state == ar::GameState::Victory) {
            if (input->IsKeyPressed(SDL_SCANCODE_SPACE) || input->IsKeyPressed(SDL_SCANCODE_RETURN)) {
                if (ReloadLevel()) SetState(ar::GameState::Playing);
            }
            return;
        }

        bridge->Update(dt);

        auto* player = bridge->GetPlayer();
        if (player) {
            auto* t = player->GetComponent<ar::Transform>();
            if (t) {
                GetCamera()->Follow(t->Position, 4.0f, dt);
                if (t->Position.y > 700.0f)
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
            0.15f,
            { 0.05f, 0.05f, 0.12f, 1.0f });
    }

    void DrawBanner(const glm::vec2& center, const glm::vec2& size,
                    const glm::vec4& color, const glm::vec4& accent) {
        auto* r = GetRenderer();
        r->DrawQuad(center, size, color);
        r->DrawQuad({ center.x, center.y + size.y * 0.35f },
                    { size.x * 0.85f, size.y * 0.12f }, accent);
    }

    void DrawHealthBar(const glm::vec2& topLeft, float width, float height) {
        if (!bridge) return;
        auto* r = GetRenderer();

        const int hp = bridge->GetPlayerHealth();
        const int maxHp = bridge->GetPlayerMaxHealth();
        if (maxHp <= 0) return;

        const float pct = (float)hp / (float)maxHp;
        const glm::vec2 barCenter = { topLeft.x + width * 0.5f, topLeft.y + height * 0.5f };

        // Outer border
        r->DrawQuad(barCenter, { width + 6.0f, height + 6.0f }, { 0.4f, 0.4f, 0.5f, 0.9f });
        // Inner background
        r->DrawQuad(barCenter, { width, height }, { 0.05f, 0.05f, 0.08f, 0.95f });

        if (pct > 0.0f) {
            glm::vec4 fillColor;
            if (pct > 0.5f)       fillColor = { 0.2f, 0.9f, 0.3f, 1.0f };
            else if (pct > 0.25f) fillColor = { 1.0f, 0.75f, 0.15f, 1.0f };
            else                  fillColor = { 1.0f, 0.2f, 0.2f, 1.0f };

            const float innerWidth = (width - 4.0f) * pct;
            const float outerLeft = barCenter.x - width * 0.5f;
            const float fillCenterX = outerLeft + 2.0f + innerWidth * 0.5f;
            r->DrawQuad({ fillCenterX, barCenter.y },
                        { innerWidth, height - 4.0f }, fillColor);
        }

        // Tick marks per HP point
        for (int i = 1; i < maxHp; ++i) {
            float t = (float)i / (float)maxHp;
            float tx = topLeft.x + width * t;
            r->DrawQuad({ tx, barCenter.y }, { 1.5f, height - 4.0f },
                        { 0.0f, 0.0f, 0.0f, 0.6f });
        }
    }

    void OnRender() override {
        auto* cam = GetCamera();
        const glm::vec2 camPos = cam->GetPosition();
        const float w = 800.0f;
        const float h = 600.0f;
        auto* r = GetRenderer();

        const ar::GameState state = GetState();

        // ---------------- TITLE SCREEN ----------------
        if (state == ar::GameState::TitleScreen) {
            r->DrawScreenOverlay(camPos, w, h, { 0.02f, 0.02f, 0.06f, 0.85f });

            if (titleTexture.GetID()) {
                ar::Sprite s;
                s.Position = camPos;
                s.Size = { w * 0.85f, h * 0.5f };
                s.Tex = &titleTexture;
                r->DrawSprite(s);
            } else {
                DrawBanner(camPos, { w * 0.85f, h * 0.35f },
                           { 0.05f, 0.08f, 0.18f, 0.95f },
                           { 0.3f, 0.7f, 1.0f, 1.0f });
                r->DrawQuad({ camPos.x, camPos.y - h * 0.35f },
                            { w * 0.6f, 30.0f }, { 0.4f, 0.85f, 1.0f, 0.9f });
                r->DrawQuad({ camPos.x, camPos.y - h * 0.12f },
                            { w * 0.5f, 20.0f }, { 1.0f, 0.4f, 0.4f, 0.9f });
            }

            float pulse = 0.6f + 0.4f * std::sin(m_Elapsed * 4.0f);
            r->DrawQuad({ camPos.x, camPos.y + h * 0.25f },
                        { w * 0.5f, 24.0f }, { 1.0f, 0.95f, 0.4f, pulse });
            r->DrawQuad({ camPos.x, camPos.y + h * 0.36f },
                        { w * 0.65f, 14.0f }, { 0.7f, 0.7f, 0.85f, 0.7f });
            return;
        }

        // ---------------- PLAYING ----------------
        if (state == ar::GameState::Playing && bridge) {
            DrawHealthBar({ camPos.x - w * 0.45f, camPos.y - h * 0.44f }, 240.0f, 26.0f);
            const int coins = bridge->GetCoinsCollected();
            r->DrawQuad({ camPos.x - w * 0.45f + 30.0f, camPos.y - h * 0.40f + 22.0f },
                        { 40.0f + coins * 6.0f, 12.0f }, { 1.0f, 0.85f, 0.1f, 0.95f });
        }

        // ---------------- GAME OVER ----------------
        if (state == ar::GameState::GameOver) {
            r->DrawScreenOverlay(camPos, w, h, { 0.15f, 0.0f, 0.0f, 0.65f });
            DrawBanner(camPos, { w * 0.7f, h * 0.28f },
                       { 0.35f, 0.05f, 0.05f, 0.95f },
                       { 0.95f, 0.15f, 0.15f, 1.0f });
            r->DrawQuad({ camPos.x, camPos.y + h * 0.22f },
                        { w * 0.55f, 18.0f }, { 1.0f, 1.0f, 1.0f, 0.85f });
            float pulse = 0.5f + 0.5f * std::sin(m_Elapsed * 3.0f);
            r->DrawQuad({ camPos.x, camPos.y + h * 0.34f },
                        { w * 0.4f, 14.0f }, { 1.0f, 0.9f, 0.3f, pulse });
        }

        // ---------------- VICTORY ----------------
        if (state == ar::GameState::Victory) {
            r->DrawScreenOverlay(camPos, w, h, { 0.0f, 0.08f, 0.15f, 0.6f });
            DrawBanner(camPos, { w * 0.75f, h * 0.3f },
                       { 0.05f, 0.35f, 0.15f, 0.95f },
                       { 0.2f, 0.95f, 0.35f, 1.0f });
            const int coins = bridge ? bridge->GetCoinsCollected() : 0;
            r->DrawQuad({ camPos.x, camPos.y + h * 0.1f },
                        { 100.0f + coins * 6.0f, 22.0f }, { 1.0f, 0.9f, 0.2f, 1.0f });
            float pulse = 0.5f + 0.5f * std::sin(m_Elapsed * 3.0f);
            r->DrawQuad({ camPos.x, camPos.y + h * 0.28f },
                        { w * 0.5f, 16.0f }, { 1.0f, 1.0f, 1.0f, pulse });
        }
    }

    void OnShutdown() override {
        delete bridge;
        bridge = nullptr;
    }
};

int main() {
    ScriptGame game;
    if (!game.Init("STARFALL — Reactor Breach", 800, 600))
        return -1;
    game.Run();
    return 0;
}