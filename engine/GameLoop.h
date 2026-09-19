#pragma once
#include <cstdint>

namespace ar {

    enum class GameState {
        TitleScreen,
        Playing,
        GameOver,
        Victory
    };

    class Window;
    class Timer;
    class Renderer;
    class Input;
    class Scene;
    class Camera;
    class Audio;

    class GameLoop {
    public:
        GameLoop();
        ~GameLoop();

        bool Init(const char* title, int width, int height);
        void Run();
        void Shutdown();

        virtual void OnInit() {}
        virtual void OnUpdate(float dt) {}
        virtual void OnPreRender() {}
        virtual void OnRender() {}
        virtual void OnShutdown() {}

        GameState GetState() const { return m_State; }
        void SetState(GameState state) { m_State = state; }

        Window* GetWindow() const { return m_Window; }
        Timer* GetTimer() const { return m_Timer; }
        Renderer* GetRenderer() const { return m_Renderer; }
        Input* GetInput() const { return m_Input; }
        Scene* GetScene() const { return m_Scene; }
        Camera* GetCamera() const { return m_Camera; }
        Audio* GetAudio() const { return m_Audio; }

        void SetTargetFPS(uint32_t fps) { m_TargetFPS = fps; }

    protected:
        Window* m_Window;
        Timer* m_Timer;
        Renderer* m_Renderer;
        Input* m_Input;
        Scene* m_Scene;
        Camera* m_Camera;
        Audio* m_Audio;

        bool m_Running;
        uint32_t m_TargetFPS;
        GameState m_State = GameState::TitleScreen;
    };

} // namespace ar