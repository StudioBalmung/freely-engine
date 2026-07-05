# Freely Engine Tutorial

Welcome to Freely Engine! This tutorial covers how to write a simple application, work with the Entity-Component-System (ECS) architecture, and manage build cache.

---

## 1. Engine Initialization

To start using Freely Engine, you subclass `Freely::Engine` and override its lifecycle methods:

- `OnInit()`: Fired once when the engine initializes. Register your systems and load scenes here.
- `OnUpdate(float dt)`: Fired every frame for game logic.
- `OnRender()`: Fired every frame to clear screen buffers and submit draws.
- `OnShutdown()`: Fired once before exit.

Here is a minimal application template:

```cpp
#include <Freely/Freely.h>
#include <Freely/ECS/Systems/RenderSystem.h>
#include <Freely/ECS/Systems/Render2DSystem.h>

class SandboxApp : public Freely::Engine {
public:
    SandboxApp() : Freely::Engine(BuildConfig()) {}

protected:
    void OnInit() override {
        FL_ENGINE_INFO("Initializing Sandbox Application...");

        // Create scene
        auto scene = std::make_shared<Freely::Scene>("MyScene");

        // Register default render pipelines
        scene->RegisterSystem(std::make_shared<Freely::RenderSystem>());
        scene->RegisterSystem(std::make_shared<Freely::Render2DSystem>());

        // Create an entity
        auto cameraEntity = scene->CreateEntity("MainCamera");
        auto& cam = cameraEntity.AddComponent<Freely::CameraComponent>();
        cam.Primary = true;
        cam.Projection = Freely::ProjectionType::Perspective;

        SetActiveScene(scene);
    }

    void OnUpdate(float dt) override {
        // Run update logic here
    }

    void OnRender() override {
        auto& renderer = GetRenderer();
        renderer.SetClearColor({ 0.05f, 0.05f, 0.07f, 1.0f });
        renderer.Clear();
    }

private:
    static Freely::EngineConfig BuildConfig() {
        Freely::EngineConfig cfg;
        cfg.windowTitle = "Freely Sandbox";
        cfg.windowWidth  = 1280;
        cfg.windowHeight = 720;
        cfg.vsync        = true;
        cfg.enable3D     = true;
        cfg.enable2D     = true;
        return cfg;
    }
};

int main() {
    SandboxApp app;
    app.Run();
    return 0;
}
```

---

## 2. Working with ECS

Freely Engine uses **EnTT** as its backend. All components are plain-old-data (POD) structures.

### Standard Components
- **TagComponent**: Holds name, tag string, layer ID, and active flags.
- **TransformComponent**: Stores `Position` (glm::vec3), `Rotation` (glm::quat), and `Scale` (glm::vec3).
- **CameraComponent**: Stores FOV, projection type (Perspective/Orthographic), and aspect ratio.
- **LightComponent**: Defines light type (Directional, Point, Spot), color, intensity, and shadow maps.

### Creating and Querying Entities

```cpp
// Create an entity with components
auto player = scene->CreateEntity("Player");
player.AddComponent<Freely::TransformComponent>(
    glm::vec3(0.0f, 1.0f, 0.0f),
    glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
    glm::vec3(1.0f, 1.0f, 1.0f)
);

// Query components
if (player.HasComponent<Freely::TransformComponent>()) {
    auto& transform = player.GetComponent<Freely::TransformComponent>();
    transform.Position.x += 1.0f; // Move right
}
```

---

## 3. Storage Optimization & Build Cache Cleaning

Since building standard C++ engines creates large build caches (`build/` folder), you can automatically clean up the build folder after a successful compilation to save local drive space.

### Cleaning Command (PowerShell)
Every time you compile the project, run:

```powershell
# Configure and build
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release

# After verification/execution, clean up the cache to free space
Remove-Item -Recurse -Force build
```
