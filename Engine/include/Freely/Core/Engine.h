#pragma once

#include "Freely/Config/EngineConfig.h"
#include "Freely/Core/Window.h"
#include "Freely/Renderer/Renderer.h"
#include "Freely/Core/Input.h"
#include "Freely/Core/Logger.h"
#include "Freely/Asset/AssetManager.h"
#include "Freely/Physics/IPhysicsBackend.h"
#include "Freely/Plugin/PluginManager.h"
#include "Freely/RHI/IRenderDevice.h"
#include "Freely/ECS/Scene.h"

#include <memory>

namespace Freely {

/// Top-level engine class. Owns subsystems and runs the main loop.
/// Configuration is fully data-driven via EngineConfig (no hardcoded values).
class Engine {
public:
    explicit Engine(EngineConfig config = EngineConfig::Defaults());
    virtual ~Engine();

    /// Load configuration from disk before constructing the Engine.
    static EngineConfig LoadConfig(const std::string& path = "freely.config");

    void Run();
    void Shutdown();

    void SetActiveScene(const std::shared_ptr<Scene>& scene);
    std::shared_ptr<Scene> GetActiveScene() const { return m_ActiveScene; }

    Window&                       GetWindow()        { return *m_Window; }
    Renderer&                     GetRenderer()      { return *m_Renderer; }
    InputManager&                 GetInput()         { return *m_Input; }
    RHI::IRenderDevice&           GetRenderDevice()  { return *m_RenderDevice; }
    Physics::IPhysicsBackend3D*   GetPhysics3D()     { return m_Physics3D.get(); }
    Physics::IPhysicsBackend2D*   GetPhysics2D()     { return m_Physics2D.get(); }
    Plugin::PluginManager&        GetPluginManager() { return *m_Plugins; }
    const EngineConfig&           GetConfig() const  { return m_Config; }

    static Engine& Get() { return *s_Instance; }

    float GetDeltaTime() const { return m_DeltaTime; }
    float GetTime() const      { return m_Time; }
    bool  IsRunning() const    { return m_Running; }

protected:
    virtual void OnInit() {}
    virtual void OnUpdate(float dt) {}
    virtual void OnRender() {}
    virtual void OnShutdown() {}

private:
    static Engine* s_Instance;

    std::unique_ptr<Window>                     m_Window;
    std::unique_ptr<Renderer>                   m_Renderer;
    std::unique_ptr<InputManager>               m_Input;
    std::unique_ptr<RHI::IRenderDevice>         m_RenderDevice;
    std::unique_ptr<Physics::IPhysicsBackend3D> m_Physics3D;
    std::unique_ptr<Physics::IPhysicsBackend2D> m_Physics2D;
    std::unique_ptr<Plugin::PluginManager>      m_Plugins;
    std::shared_ptr<Scene>                      m_ActiveScene;

    EngineConfig m_Config;
    bool         m_Running   = false;
    float        m_DeltaTime = 0.0f;
    float        m_Time      = 0.0f;
    float        m_PhysicsAccumulator = 0.0f;

    void InitSystems();
    void ShutdownSystems();
    void UpdateAudioListener(Scene& scene);
};

} // namespace Freely