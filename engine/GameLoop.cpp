#include "GameLoop.h"
#include "Window.h"
#include "Timer.h"
#include "Renderer.h"
#include "Input.h"
#include "Scene.h"
#include "Camera.h"
#include "Audio.h"

#include <iostream>

namespace ar {

    GameLoop::GameLoop()
        : m_Window(nullptr),
        m_Timer(nullptr),
        m_Renderer(nullptr),
        m_Input(nullptr),
        m_Scene(nullptr),
        m_Camera(nullptr),
        m_Audio(nullptr),
        m_Running(false),
        m_TargetFPS(60)
    {
    }


    GameLoop::~GameLoop() {
        Shutdown();
    }


    bool GameLoop::Init(const char* title, int width, int height) {

        m_Window = new Window();

        if (!m_Window->Init(title, width, height)) {
            delete m_Window;
            m_Window = nullptr;
            return false;
        }


        m_Renderer = new Renderer();

        if (!m_Renderer->Init()) {
            delete m_Renderer;
            m_Renderer = nullptr;
            delete m_Window;
            m_Window = nullptr;
            return false;
        }


        m_Input = new Input();

        m_Scene = new Scene();


        m_Camera = new Camera(
            (float)width,
            (float)height
        );

        m_Camera->SetPosition({
            width * 0.5f,
            height * 0.5f
            });


        // Audio system
        m_Audio = new Audio();

        if (!m_Audio->Init()) {
            std::cout << "[AR] Audio initialization failed.\n";
        }
        else {
            std::cout << "[AR] Audio initialized.\n";
        }


        m_Timer = new Timer();

        m_Timer->Start();
        m_Timer->SetTargetFPS(m_TargetFPS);


        m_Running = true;

        OnInit();

        std::cout << "[AR] GameLoop ready.\n";

        return true;
    }



    void GameLoop::Run() {

        while (m_Running && !m_Window->ShouldClose()) {

            m_Timer->Tick();

            float dt = m_Timer->GetDeltaTime();


            m_Window->PollEvents(*m_Input);


            OnUpdate(dt);

            m_Scene->Update(dt);



            m_Renderer->Clear();

            m_Renderer->BeginScene(
                m_Camera->GetViewProjection()
            );


            m_Scene->Render(m_Renderer);

            OnRender();


            m_Renderer->EndScene();


            m_Window->SwapBuffers();


            m_Timer->DelayIfNeeded();
        }
    }



    void GameLoop::Shutdown() {

        if (!m_Running)
            return;


        OnShutdown();


        delete m_Camera;
        m_Camera = nullptr;


        delete m_Scene;
        m_Scene = nullptr;


        delete m_Input;
        m_Input = nullptr;


        delete m_Renderer;
        m_Renderer = nullptr;


        // Shutdown audio before destroying engine
        if (m_Audio) {
            m_Audio->Shutdown();
            delete m_Audio;
            m_Audio = nullptr;
        }


        delete m_Timer;
        m_Timer = nullptr;


        delete m_Window;
        m_Window = nullptr;


        m_Running = false;


        std::cout << "[AR] Shutdown.\n";
    }

} // namespace ar