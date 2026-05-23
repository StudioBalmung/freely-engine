# Freely Engine

A custom C++ 3D Game Engine built with OpenGL 4.5.

## Features

- **OpenGL 4.5 Core Profile** with DSA (Direct State Access)
- **PBR Rendering** (Cook-Torrance BRDF with GGX distribution)
- **ImGui/ImGuizmo Editor** - Dockable GUI, 3D viewport with grid and gizmos
- **AsterCore Physics** - Rigid body dynamics, collision detection (Sphere/Box/Plane)
- **Modular Architecture** - Core, Renderer, Scene, Physics, Editor modules
- **Camera System** - Perspective/Orthographic with orbit/pan editor controls
- **Mesh Primitives** - Cube, Sphere, Plane + OBJ file loading
- **Material System** - PBR materials with texture support
- **Shader Abstraction** - Compile from source or file, uniform caching
- **Framebuffer** - Off-screen rendering for viewport
- **Input System** - Keyboard, mouse, scroll with press/release detection
- **Logging** - Color-coded engine/app logging via spdlog

## Architecture

```
Proj/
├── Engine/
│   ├── include/Freely/
│   │   ├── Core/       (Engine, Window, Input, Logger)
│   │   ├── Renderer/   (Renderer, Shader, Buffer, VAO, Texture, Framebuffer)
│   │   └── Scene/      (Camera, Mesh, Material, Light)
│   └── src/            (Implementations)
├── AsterCore/
│   ├── include/AsterCore/  (PhysicsWorld, RigidBody, Collider, CollisionDetection)
│   └── src/
├── Editor/
│   ├── include/Editor/     (EditorApp)
│   └── src/                (Editor with ImGui/ImGuizmo)
├── Examples/               (Demo application)
└── vendor/                 (glad OpenGL loader)
```

## Dependencies (auto-fetched via CMake FetchContent)

- **GLFW 3.3.9** - Window/context management
- **GLM 1.0.1** - Mathematics library
- **spdlog 1.13.0** - Logging
- **stb** - Image loading (stb_image)
- **tinyobjloader** - OBJ mesh loading
- **ImGui (Docking)** - GUI library
- **ImGuizmo** - 3D Gizmo/Grid for viewport
- **glad** - OpenGL function loader (vendored)

## Build Instructions

### Prerequisites

- CMake 3.20+
- C++20 compatible compiler (MSVC 2022, GCC 12+, Clang 15+)
- Git (for FetchContent)

### Windows (Visual Studio)

```bash
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022"
cmake --build . --config Release
```

### Windows (Ninja/MinGW)

```bash
mkdir build
cd build
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

### Linux/macOS

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

### Run

```bash
# Editor
./build/bin/FreelyEditor

# Demo
./build/bin/FreelyDemo
```

## Editor Controls

| Key | Action |
|-----|--------|
| Right Mouse + Drag | Orbit camera |
| Middle Mouse + Drag | Pan camera |
| Scroll | Zoom in/out |
| W | Translate gizmo |
| E | Rotate gizmo |
| R | Scale gizmo |
| Escape | Exit |

## CMake Options

| Option | Default | Description |
|--------|---------|-------------|
| `FREELY_BUILD_EXAMPLES` | ON | Build example applications |
| `FREELY_BUILD_EDITOR` | ON | Build the Freely Editor |
| `FREELY_USE_VULKAN` | OFF | Enable Vulkan backend (WIP) |

## Extending

To create your own application, inherit from `Freely::Engine`:

```cpp
#include <Freely/Freely.h>

class MyApp : public Freely::Engine {
public:
    MyApp() : Engine({"My App", 1920, 1080, true, false}) {}

protected:
    void OnInit() override { /* Setup */ }
    void OnUpdate(float dt) override { /* Logic */ }
    void OnRender() override { /* Drawing */ }
    void OnShutdown() override { /* Cleanup */ }
};

int main() {
    MyApp app;
    app.Run();
    return 0;
}
```

## License

MIT
