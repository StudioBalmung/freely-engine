#include "Freely/Core/Engine.h"
#include "Freely/Core/Window.h"
#include "Freely/Core/Input.h"
#include "Freely/Core/Logger.h"
#include "Freely/Renderer/Renderer.h"
#include "Freely/Renderer/Renderer3D.h"
#include "Freely/Renderer2D/Renderer2D.h"
#include "Freely/Audio/AudioEngine.h"
#include "Freely/Asset/AssetManager.h"
#include "Freely/Physics/IPhysicsBackend.h"
#include "Freely/Plugin/PluginManager.h"
#include "Freely/ECS/Scene.h"
#include "Freely/Renderer/Shader.h"

#include <GLFW/glfw3.h>

namespace Freely {

Engine* Engine::s_Instance = nullptr;

static constexpr float kFixedTimestep = 1.0f / 60.0f;
static constexpr float kMaxFrameTime  = 0.25f; // prevent spiral of death

Engine::Engine(EngineConfig config) : m_Config(std::move(config)) {
    s_Instance = this;
}
Engine::~Engine() { s_Instance = nullptr; }

void Engine::Run() {
    Logger::Init();
    FL_ENGINE_INFO("Freely Engine starting…");

    // -- Window ----------------------------------------------------------
    m_Window = std::make_unique<Window>(
        m_Config.Window.Title, m_Config.Window.Width,
        m_Config.Window.Height, m_Config.Window.VSync,
        m_Config.Window.Mode == WindowMode::Fullscreen);

    // -- Renderer (base OpenGL state) -------------------------------------
    m_Renderer = std::make_unique<Renderer>();
    m_Renderer->Init();

    // -- All pipeline systems ---------------------------------------------
    InitSystems();

    // -- Input ------------------------------------------------------------
    m_Input = std::make_unique<InputManager>(m_Window->GetNativeHandle());

    m_Window->SetResizeCallback([this](int w, int h) {
        m_Renderer->SetViewport(0, 0, w, h);
        Renderer3D::SetViewportSize((uint32_t)w, (uint32_t)h);
    });

    m_Running = true;
    OnInit();

    float lastTime    = (float)glfwGetTime();
    float accumulator = 0.0f;

    while (m_Running && !m_Window->ShouldClose()) {
        float now = (float)glfwGetTime();
        float dt  = std::min(now - lastTime, kMaxFrameTime);
        lastTime  = now;
        m_DeltaTime = dt;
        m_Time      = now;

        m_Window->PollEvents();
        m_Input->Update();
        Shader::UpdateHotReload();

        // -- Fixed timestep (physics + deterministic systems) -------------
        accumulator += dt;
        while (accumulator >= kFixedTimestep) {
            if (m_ActiveScene) m_ActiveScene->FixedUpdate(kFixedTimestep);
            accumulator -= kFixedTimestep;
        }

        // -- Variable update -----------------------------------------------
        OnUpdate(dt);
        if (m_ActiveScene) m_ActiveScene->Update(dt);

        // -- Late update (rendering) ----------------------------------------
        if (m_ActiveScene) m_ActiveScene->LateUpdate(dt);

        // -- Audio listener sync (uses primary camera entity) --------------
        if (m_ActiveScene)
            UpdateAudioListener(*m_ActiveScene);

        OnRender();

        m_Window->SwapBuffers();
    }

    OnShutdown();
    ShutdownSystems();
    m_Renderer->Shutdown();
    FL_ENGINE_INFO("Freely Engine shutdown complete.");
}

void Engine::UpdateAudioListener(Scene& scene) {
    auto& reg = scene.GetRegistry().GetEnttRegistry();
    auto view = reg.view<TransformComponent, CameraComponent>();
    for (auto e : view) {
        auto [tf, cc] = view.get<TransformComponent, CameraComponent>(e);
        if (!cc.Primary) continue;
        AudioEngine::Update(tf.Position, tf.GetForward(), tf.GetUp());
        break;
    }
}

void Engine::InitSystems() {
    AssetManager::Init(m_Config.Assets.AssetRoot);
    // Initialize renderers based on config - for now always initialize both
    // since we don't have enable3D/enable2d flags in EngineConfig
    Renderer3D::Init();
    Renderer2D::Init();
    AudioEngine::Init();
    FL_ENGINE_INFO("Pipeline systems: 3D=1 2D=1 Audio=1");
}

void Engine::ShutdownSystems() {
    AudioEngine::Shutdown();
    Renderer2D::Shutdown();
    Renderer3D::Shutdown();
    AssetManager::Shutdown();
}

void Engine::SetActiveScene(const std::shared_ptr<Scene>& scene) {
    m_ActiveScene = scene;
}

void Engine::Shutdown() { m_Running = false; }

} // namespace Freely