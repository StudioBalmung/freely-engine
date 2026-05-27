#include "Freely/Core/Engine.h"
#include "Freely/Core/Window.h"
#include "Freely/Core/Input.h"
#include "Freely/Core/Logger.h"
#include "Freely/Renderer/Renderer.h"

#include <GLFW/glfw3.h>

namespace Freely {

Engine* Engine::s_Instance = nullptr;

Engine::Engine(const EngineConfig& config)
    : m_Config(config)
{
    s_Instance = this;
}

Engine::~Engine() {
    s_Instance = nullptr;
}

void Engine::Run() {
    Logger::Init();
    FL_ENGINE_INFO("Freely Engine v0.1.0 starting...");

    m_Window = std::make_unique<Window>(
        m_Config.windowTitle,
        m_Config.windowWidth,
        m_Config.windowHeight,
        m_Config.vsync,
        m_Config.fullscreen
    );

    m_Renderer = std::make_unique<Renderer>();
    m_Renderer->Init();

    m_Input = std::make_unique<InputManager>(m_Window->GetNativeHandle());

    m_Window->SetResizeCallback([this](int width, int height) {
        m_Renderer->SetViewport(0, 0, width, height);
    });

    m_Running = true;
    OnInit();

    float lastTime = static_cast<float>(glfwGetTime());

    while (m_Running && !m_Window->ShouldClose()) {
        float currentTime = static_cast<float>(glfwGetTime());
        m_DeltaTime = currentTime - lastTime;
        lastTime = currentTime;
        m_Time = currentTime;

        m_Window->PollEvents();
        m_Input->Update();

        OnUpdate(m_DeltaTime);
        OnRender();

        m_Window->SwapBuffers();
    }

    OnShutdown();
    m_Renderer->Shutdown();
    FL_ENGINE_INFO("Freely Engine shutdown complete.");
}

void Engine::Shutdown() {
    m_Running = false;
}

} // namespace Freely
