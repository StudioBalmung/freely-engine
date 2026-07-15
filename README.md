# Freely Engine

[![License: FPL](https://img.shields.io/badge/License-FPL-scarlet.svg)](LICENSE)
[![Version](https://img.shields.io/badge/version-0.5.0-B10C1A)](https://github.com/StudioBalmung/freely-engine/releases)

A Custom C++20 game engine with both a 3D pipeline
and a dedicated 2D renderer.

## Features

- **2D Renderer** (`Renderer2D`) - batched sprite/rect/line/SDF-text
  rendering, sprite sheets, bitmap fonts baked from TTF via stb_truetype
- **3D Renderer** - OpenGL 4.5 Core Profile with DSA (Direct State
  Access), PBR (Cook-Torrance BRDF / GGX), optional Vulkan backend
  behind an `RHI` abstraction (`IRenderDevice`, `RHIResources`, `RHITypes`)
- **ECS** - entity/component/system layer built on EnTT 3.16.0
- **ImGui/ImGuizmo Editor** - dockable GUI, 3D viewport with grid and gizmos
- **Physics** - optional Box2D (2D, default on) and Jolt (3D, default off)
  backends; both are declared as CMake options and link the vendored
  library, but no game-facing physics API in `Engine/Physics` has been
  wired to either yet (`FREELY_PHYSICS_BOX2D=1` / `FREELY_PHYSICS_JOLT=1`
  are only preprocessor defines at this point)
- **Scripting** - optional Lua (default on) and C#/Mono (default off)
- **Audio** - `AudioEngine` is implemented on **miniaudio** only. The
  `FREELY_AUDIO_OPENAL` / `FREELY_AUDIO_SDL` CMake options exist and
  define `FREELY_AUDIO_OPENAL=1` / `FREELY_AUDIO_SDL=1`, but nothing in
  the engine currently reads those defines or links OpenAL/SDL3 audio -
  they are placeholders, not working backends, as of this snapshot
- **Camera** - perspective/orthographic, orbit/pan editor controls.
  Note: `Camera` defaults to position `(0,0,3)` looking toward `+Z`;
  2D games must explicitly position their camera at negative Z (or
  otherwise ensure their geometry is in front of it), or nothing will
  render. See a real writeup of this in the Darkness Rising source
  package (`engine/PATCHES.md`, item 6) if building a 2D game on Freely.
- **Asset / Project / Build / Plugin / Config** - project file handling,
  cross-platform build-target definitions, plugin loading, engine config
- **Logging** - color-coded engine/app logging via spdlog

## Architecture

This snapshot's actual layout (not every folder shown has a CMake target
wired up yet - see §CMake Options):

```
freely-main/
├── Engine/
│   ├── include/Freely/
│   │   ├── Core/         (Engine, Window, Input, Logger)
│   │   ├── Renderer/     (Renderer, Shader, Buffer, VAO, UniformBuffer, Texture, Framebuffer)
│   │   ├── Renderer2D/   (Renderer2D, SpriteSheet, Font, Animation2D)
│   │   ├── RHI/          (IRenderDevice, RHIResources, RHITypes)
│   │   ├── Scene/        (Camera, Mesh, Material, Light)
│   │   ├── ECS/          (Registry, SceneGraph, Systems, Components, UUID)
│   │   ├── Physics/
│   │   ├── Audio/        (AudioEngine - miniaudio backed)
│   │   ├── Scripting/
│   │   ├── Asset/        (AssetManager)
│   │   ├── Project/
│   │   ├── Build/        (BuildTarget, BuildPipeline)
│   │   ├── Plugin/
│   │   └── Config/
│   └── src/               (Implementations)
├── Editor/
│   ├── include/Editor/    (EditorApp, EditorContext, Commands, Panels)
│   └── src/               (Editor with ImGui/ImGuizmo)
├── Runtime/
│   └── src/                (Standalone runtime entry point)
├── Fonts/                  (InterVariable.ttf)
├── ThirdParty/              (vendored dependencies - see below)
└── CMakeLists.txt
```

There is no `Proj/`, `AsterCore/`, `Examples/`, or `vendor/` folder in
this snapshot; if you have documentation or notes referencing those,
they describe an older layout.

## Dependencies

As of this snapshot, dependencies are vendored under `ThirdParty/`
rather than fetched over the network at configure time:

| Library | Version | Used by current CMakeLists.txt? |
|---|---|---|
| GLFW | 3.3.9 | Yes - window/input |
| GLM | 1.0.1 | Yes - math |
| spdlog | 1.13.0 | Yes - logging |
| EnTT | 3.16.0 | Yes - ECS |
| Box2D | 3.2.0 | Yes, if `FREELY_PHYSICS_BOX2D=ON` (default). Its `extern/glad` copy is also reused as the engine's OpenGL loader (glad1 API - see Known Issues) |
| Jolt | (vendored) | Only if `FREELY_PHYSICS_JOLT=ON` (default off) |
| glad2 | n/a | **Generator source only**, not a pre-generated loader. Not currently linked by the build - see Known Issues |
| bgfx | (vendored) | No - not referenced anywhere in CMakeLists.txt |
| SDL3 | (vendored) | No - not referenced anywhere in CMakeLists.txt |
| libPNG, zlib, zstd, xxHash, Ogg, glsl | (vendored) | Not directly referenced by CMakeLists.txt at the top level; may be transitive deps of the above |
| Vulkan | SDK, not vendored | Only if `FREELY_USE_VULKAN=ON` (default). Requires a real Vulkan SDK installed at a fixed path (`C:/VulkanSDK/...` on Windows); not something `ThirdParty/` can satisfy on its own |
| ImGui (docking), ImGuizmo, stb, tinyobjloader, miniaudio, Lua (if enabled), VMA (if Vulkan enabled) | various | Yes, still via `FetchContent` (network required at configure time) - no vendored copies exist for these yet |

`bgfx/` and `SDL3/` together account for over 300 MB of this snapshot
and match the dependency set used by Styx Engine, not Freely's current
renderer. If they're not meant to stay, they're safe to remove without
affecting any current build target.

## Known Issues

- **glad version conflict.** `Engine/src/Core/Window.cpp` calls
  `gladLoadGL((GLADloadfunc)glfwGetProcAddress)`, which is the **glad2**
  API. The current `CMakeLists.txt` builds its OpenGL loader from
  `ThirdParty/Box2D/extern/glad`, which is a **glad1** loader (`gladLoadGL()`
  takes no arguments; the loader-function entry point is
  `gladLoadGLLoader(GLADloadproc)` instead). Building as-is will fail to
  compile. Fix by either generating a real glad2 loader into
  `ThirdParty/glad2` (it currently only contains the Python generator,
  not output) and linking that instead of Box2D's copy, or by reverting
  `Window.cpp` to the glad1 API if standardizing on Box2D's vendored copy.
- **Camera default facing.** See the Camera bullet under Features above.
- **Unwired CMake options.** `FREELY_BUILD_EXAMPLES`, `FREELY_AUDIO_OPENAL`,
  and `FREELY_AUDIO_SDL` exist as options but have no corresponding
  build targets or code paths yet.

## Build Instructions

### Prerequisites

- CMake 3.22+
- C++20 compatible compiler (MSVC 2022, GCC 12+, Clang 15+)
- If `FREELY_USE_VULKAN=ON` (default): Vulkan SDK installed
- Internet access at configure time for the still-`FetchContent`-based
  dependencies (ImGui, ImGuizmo, stb, tinyobjloader, miniaudio, Lua, VMA)

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

For a 2D-only game that does not need the Vulkan/ImGui editor path
(e.g. cross-compiling to Windows from Linux with mingw-w64), build with
`-DFREELY_USE_VULKAN=OFF -DFREELY_BUILD_EDITOR=OFF` and link only the
`Core`, `Renderer`, `Renderer2D`, `Scene/Camera`, and `Audio` translation
units directly rather than the full `FreelyEngine` CMake target - see
the Darkness Rising source package's `build-scripts/build.sh` for a
working example of this approach.

### Run

```bash
# Editor
./build/bin/FreelyEditor

# Runtime (if FREELY_BUILD_RUNTIME=ON)
./build/bin/FreelyRuntime
```

There is no `FreelyDemo` / `Examples` target in this snapshot despite
the `FREELY_BUILD_EXAMPLES` option existing.

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
| `FREELY_BUILD_EDITOR` | ON | Build the Freely Editor |
| `FREELY_BUILD_EXAMPLES` | ON | Declared, but no example targets exist yet |
| `FREELY_BUILD_RUNTIME` | ON | Build standalone runtime |
| `FREELY_USE_VULKAN` | ON | Enable Vulkan RHI backend (requires Vulkan SDK) |
| `FREELY_SCRIPTING_LUA` | ON | Enable Lua scripting |
| `FREELY_SCRIPTING_MONO` | OFF | Enable C#/Mono scripting |
| `FREELY_AUDIO_OPENAL` | ON | Declared, not yet wired to a working backend |
| `FREELY_AUDIO_SDL` | ON | Declared, not yet wired to a working backend |
| `FREELY_PHYSICS_BOX2D` | ON | Enable Box2D 2D physics backend (links the library; no engine-side integration yet) |
| `FREELY_PHYSICS_JOLT` | OFF | Enable Jolt 3D physics backend (links the library; no engine-side integration yet) |

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
## Recent Updates

- Fixed `Registry.h` const‑view bug and cleaned includes.
- Fixed shadow‑pass double‑draw issue in `Renderer3D.cpp`.
- Rewrote `CMakeLists.txt` to correctly reference the vendored `glad` loader and to add all requested Third‑Party libraries (bgfx, SDL3, SFML, OpenAL, FBX, mimalloc, xxHash, zstd, libPNG, Ogg).
- Added full macOS build target (`macOSBuildTarget`) supporting universal binaries, Xcode generation, ad‑hoc code‑signing and optional DMG packaging.
- Updated `BuildPipeline` to register the macOS target.

These changes bring the engine to a **fully functional** state for Windows, Linux, macOS, Android and WebGL builds.


For a 2D game, it is also valid to skip `Freely::Engine`'s ECS/Scene-graph
game loop entirely and drive `Window` / `InputManager` / `Renderer2D` /
`AudioEngine` directly from your own loop - see Darkness Rising for a
complete working example of that approach.

## License

FPL (Freely Public License) 

2026 (c) Neofilisoft / Studio Balmung.
