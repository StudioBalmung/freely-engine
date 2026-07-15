// Freely Engine - Standalone Runtime

#include <Freely/Freely.h>
#include <Freely/ECS/Systems/RenderSystem.h>
#include <Freely/ECS/Systems/Render2DSystem.h>
#include <Freely/ECS/SceneSerializer.h>

#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

// Runtime application 
class RuntimeApp : public Freely::Engine {
public:
    explicit RuntimeApp(const std::string& projectPath)
        : Freely::Engine(BuildConfig(projectPath))
        , m_ProjectPath(projectPath)
    {}

protected:
    void OnInit() override {
        FL_ENGINE_INFO("Runtime: loading project '{}'", m_ProjectPath);

        auto scene = std::make_shared<Freely::Scene>("RuntimeScene");

        // Register systems
        scene->RegisterSystem(std::make_shared<Freely::RenderSystem>());
        scene->RegisterSystem(std::make_shared<Freely::Render2DSystem>());

        // Load the first scene from the project
        fs::path scenePath = fs::path(m_ProjectPath) / "Assets" / "Scenes" / "Main.freely";
        if (fs::exists(scenePath)) {
            Freely::SceneSerializer ser(scene);
            ser.Deserialize(scenePath.string());
            FL_ENGINE_INFO("Runtime: scene loaded from '{}'", scenePath.string());
        } else {
            FL_ENGINE_WARN("Runtime: no scene found at '{}', starting empty.", scenePath.string());
        }

        SetActiveScene(scene);
    }

    void OnUpdate(float dt) override {
        // Input handling, game logic driven by NativeScript / Lua components
    }

    void OnRender() override {
        auto& renderer = GetRenderer();
        renderer.SetClearColor({0.05f, 0.05f, 0.07f, 1.0f});
        renderer.Clear();
    }

    void OnShutdown() override {
        FL_ENGINE_INFO("Runtime: shutdown.");
    }

private:
    static Freely::EngineConfig BuildConfig(const std::string& projectPath) {
        Freely::EngineConfig cfg;
        cfg.windowTitle = "Freely Runtime";
        cfg.windowWidth  = 1600;
        cfg.windowHeight = 900;
        cfg.vsync        = true;
        cfg.enable3D     = true;
        cfg.enable2D     = true;
        // TODO: read project.json for title/resolution overrides
        return cfg;
    }

    std::string m_ProjectPath;
};

//  Entry point 
int main(int argc, char** argv) {
    std::string projectPath = ".";
    if (argc > 1) projectPath = argv[1];

    RuntimeApp app(projectPath);
    app.Run();
    return 0;
}
