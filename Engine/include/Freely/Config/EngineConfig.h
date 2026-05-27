#pragma once

#include <string>
#include <unordered_map>
#include <variant>
#include <filesystem>

namespace Freely {

/// Render backend selection.
enum class RenderAPI {
    OpenGL,
    Vulkan,
    DirectX12,
    Auto
};

/// 3D physics backend selection.
enum class PhysicsBackend3D {
    AsterCore,
    Jolt,
    PhysX,
    None
};

/// 2D physics backend selection.
enum class PhysicsBackend2D {
    Box2D,
    None
};

/// Logging level (matches spdlog levels).
enum class LogLevel {
    Trace,
    Debug,
    Info,
    Warn,
    Error,
    Critical,
    Off
};

/// Window display mode.
enum class WindowMode {
    Windowed,
    Borderless,
    Fullscreen
};

/// Window-specific configuration.
struct WindowConfig {
    std::string Title;
    int Width;
    int Height;
    WindowMode Mode;
    bool Resizable;
    bool VSync;
    int MSAA; // 0,2,4,8,16
};

/// Renderer configuration.
struct RendererConfig {
    RenderAPI API;
    bool EnableHDR;
    bool EnableShadows;
    int ShadowMapResolution;
    float MaxAnisotropy;
    bool EnableValidationLayers;
};

/// Physics configuration.
struct PhysicsConfig {
    PhysicsBackend3D Backend3D;
    PhysicsBackend2D Backend2D;
    float FixedTimeStep;
    int MaxSubSteps;
    float Gravity3DX, Gravity3DY, Gravity3DZ;
    float Gravity2DX, Gravity2DY;
    int WorkerThreads;
};

/// Asset/streaming configuration.
struct AssetConfig {
    std::string AssetRoot;
    std::string ShaderRoot;
    std::string PluginRoot;
    std::string CacheRoot;
    int StreamingBudgetMB;
    bool AsyncLoading;
};

/// Logging configuration.
struct LoggingConfig {
    LogLevel EngineLevel;
    LogLevel AppLevel;
    std::string LogFile;
    bool LogToFile;
    bool LogToConsole;
    bool ColorizedOutput;
};

/// Editor configuration.
struct EditorConfig {
    std::string ProjectRoot;
    std::string LayoutFile;
    float WindowRounding;
    float FrameRounding;
    float GrabRounding;
    std::string Theme;
    std::string FontPath;
    float FontSize;
    bool EnableDocking;
    bool EnableViewports;
};

/// Complete engine configuration loaded from disk.
class EngineConfig {
public:
    WindowConfig    Window;
    RendererConfig  Renderer;
    PhysicsConfig   Physics;
    AssetConfig     Assets;
    LoggingConfig   Logging;
    EditorConfig    Editor;

    /// Load configuration from a config file. Returns default config on failure.
    static EngineConfig LoadFromFile(const std::filesystem::path& path);

    /// Save current configuration back to disk.
    bool SaveToFile(const std::filesystem::path& path) const;

    /// Returns the default config baked into the engine binary.
    static EngineConfig Defaults();

    /// Resolve a path relative to the asset root.
    std::filesystem::path ResolveAsset(const std::string& relative) const;
};

/// String <-> enum helpers for serialization.
RenderAPI         RenderAPIFromString(const std::string& s);
const char*       RenderAPIToString(RenderAPI v);
PhysicsBackend3D  PhysicsBackend3DFromString(const std::string& s);
const char*       PhysicsBackend3DToString(PhysicsBackend3D v);
PhysicsBackend2D  PhysicsBackend2DFromString(const std::string& s);
const char*       PhysicsBackend2DToString(PhysicsBackend2D v);
LogLevel          LogLevelFromString(const std::string& s);
const char*       LogLevelToString(LogLevel v);
WindowMode        WindowModeFromString(const std::string& s);
const char*       WindowModeToString(WindowMode v);

} // namespace Freely
