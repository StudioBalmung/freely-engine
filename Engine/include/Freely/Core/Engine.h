#pragma once
// Freely Engine - Engine core
#include <string>
#include <memory>

namespace Freely {

class Scene;

struct EngineConfig {
    std::string windowTitle   = "Freely Engine";
    std::string projectRoot   = ".";
    int  windowWidth          = 1280;
    int  windowHeight         = 720;
    bool vsync                = true;
    bool fullscreen           = false;
    bool enable3D             = true;
    bool enable2D             = true;
    bool enableAudio          = true;
    bool enablePhysics        = true;
};

class Window;
class Renderer;
class InputManager;

class Engine {
public:
    explicit Engine(const EngineConfig& config = {});
    ~Engine();

    void Run();
    void Shutdown();

    Window&       GetWindow()   { return *m_Window; }
    Renderer&     GetRenderer() { return *m_Renderer; }
    InputManager& GetInput()    { return *m_Input; }

    static Engine& Get() { return *s_Instance; }

    float GetDeltaTime() const { return m_DeltaTime; }
    float GetTime()      const { return m_Time; }
    bool  IsRunning()    const { return m_Running; }

    std::shared_ptr<Scene> GetActiveScene()                  { return m_ActiveScene; }
    void SetActiveScene(std::shared_ptr<Scene> s)            { m_ActiveScene = s; }

protected:
    virtual void OnInit()           {}
    virtual void OnUpdate(float dt) {}
    virtual void OnRender()         {}
    virtual void OnShutdown()       {}

private:
    void InitSystems();
    void ShutdownSystems();
    void UpdateAudioListener(Scene& scene);

    static Engine* s_Instance;

    std::unique_ptr<Window>       m_Window;
    std::unique_ptr<Renderer>     m_Renderer;
    std::unique_ptr<InputManager> m_Input;
    std::shared_ptr<Scene>        m_ActiveScene;

    EngineConfig m_Config;
    bool  m_Running   = false;
    float m_DeltaTime = 0.0f;
    float m_Time      = 0.0f;
};

} // namespace Freely
