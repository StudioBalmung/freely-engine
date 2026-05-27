#include "Freely/Config/EngineConfig.h"
#include "Freely/Core/Logger.h"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>

namespace Freely {

namespace {

std::string Trim(const std::string& s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    size_t b = s.find_last_not_of(" \t\r\n");
    return (a == std::string::npos) ? std::string() : s.substr(a, b - a + 1);
}

std::string StripQuotes(const std::string& s) {
    if (s.size() >= 2 && ((s.front() == '"' && s.back() == '"') ||
                          (s.front() == '\'' && s.back() == '\''))) {
        return s.substr(1, s.size() - 2);
    }
    return s;
}

std::string ToLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
        [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
    return s;
}

bool ParseBool(const std::string& s, bool fallback) {
    std::string v = ToLower(Trim(StripQuotes(s)));
    if (v == "true" || v == "yes" || v == "on" || v == "1") return true;
    if (v == "false" || v == "no" || v == "off" || v == "0") return false;
    return fallback;
}

int ParseInt(const std::string& s, int fallback) {
    try { return std::stoi(Trim(StripQuotes(s))); }
    catch (...) { return fallback; }
}

float ParseFloat(const std::string& s, float fallback) {
    try { return std::stof(Trim(StripQuotes(s))); }
    catch (...) { return fallback; }
}

/// Minimal INI/TOML-flat parser. Supports `[section]` and `key = value` lines.
/// Comments start with `#` or `;`.
struct FlatConfig {
    // section -> key -> value (raw string)
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> Data;

    bool Has(const std::string& section, const std::string& key) const {
        auto it = Data.find(section);
        return it != Data.end() && it->second.count(key) > 0;
    }

    std::string Get(const std::string& section, const std::string& key,
                    const std::string& fallback = {}) const {
        auto it = Data.find(section);
        if (it == Data.end()) return fallback;
        auto kit = it->second.find(key);
        if (kit == it->second.end()) return fallback;
        return kit->second;
    }

    static FlatConfig Parse(std::istream& in) {
        FlatConfig cfg;
        std::string line;
        std::string current = "";
        while (std::getline(in, line)) {
            // strip comments
            for (size_t i = 0; i < line.size(); i++) {
                if (line[i] == '#' || line[i] == ';') {
                    line = line.substr(0, i);
                    break;
                }
            }
            line = Trim(line);
            if (line.empty()) continue;

            if (line.front() == '[' && line.back() == ']') {
                current = ToLower(Trim(line.substr(1, line.size() - 2)));
                continue;
            }

            size_t eq = line.find('=');
            if (eq == std::string::npos) continue;
            std::string key = ToLower(Trim(line.substr(0, eq)));
            std::string val = Trim(line.substr(eq + 1));
            cfg.Data[current][key] = val;
        }
        return cfg;
    }
};

} // namespace

// ---------- enum string helpers ----------

RenderAPI RenderAPIFromString(const std::string& s) {
    std::string v = ToLower(StripQuotes(s));
    if (v == "opengl")    return RenderAPI::OpenGL;
    if (v == "vulkan")    return RenderAPI::Vulkan;
    if (v == "directx12" || v == "dx12" || v == "d3d12") return RenderAPI::DirectX12;
    return RenderAPI::Auto;
}
const char* RenderAPIToString(RenderAPI v) {
    switch (v) {
        case RenderAPI::OpenGL:    return "OpenGL";
        case RenderAPI::Vulkan:    return "Vulkan";
        case RenderAPI::DirectX12: return "DirectX12";
        case RenderAPI::Auto:      return "Auto";
    }
    return "Auto";
}

PhysicsBackend3D PhysicsBackend3DFromString(const std::string& s) {
    std::string v = ToLower(StripQuotes(s));
    if (v == "astercore") return PhysicsBackend3D::AsterCore;
    if (v == "jolt")      return PhysicsBackend3D::Jolt;
    if (v == "physx")     return PhysicsBackend3D::PhysX;
    return PhysicsBackend3D::None;
}
const char* PhysicsBackend3DToString(PhysicsBackend3D v) {
    switch (v) {
        case PhysicsBackend3D::AsterCore: return "AsterCore";
        case PhysicsBackend3D::Jolt:      return "Jolt";
        case PhysicsBackend3D::PhysX:     return "PhysX";
        case PhysicsBackend3D::None:      return "None";
    }
    return "None";
}

PhysicsBackend2D PhysicsBackend2DFromString(const std::string& s) {
    std::string v = ToLower(StripQuotes(s));
    if (v == "box2d") return PhysicsBackend2D::Box2D;
    return PhysicsBackend2D::None;
}
const char* PhysicsBackend2DToString(PhysicsBackend2D v) {
    return (v == PhysicsBackend2D::Box2D) ? "Box2D" : "None";
}

LogLevel LogLevelFromString(const std::string& s) {
    std::string v = ToLower(StripQuotes(s));
    if (v == "trace")    return LogLevel::Trace;
    if (v == "debug")    return LogLevel::Debug;
    if (v == "info")     return LogLevel::Info;
    if (v == "warn" || v == "warning") return LogLevel::Warn;
    if (v == "error")    return LogLevel::Error;
    if (v == "critical") return LogLevel::Critical;
    if (v == "off")      return LogLevel::Off;
    return LogLevel::Info;
}
const char* LogLevelToString(LogLevel v) {
    switch (v) {
        case LogLevel::Trace:    return "Trace";
        case LogLevel::Debug:    return "Debug";
        case LogLevel::Info:     return "Info";
        case LogLevel::Warn:     return "Warn";
        case LogLevel::Error:    return "Error";
        case LogLevel::Critical: return "Critical";
        case LogLevel::Off:      return "Off";
    }
    return "Info";
}

WindowMode WindowModeFromString(const std::string& s) {
    std::string v = ToLower(StripQuotes(s));
    if (v == "fullscreen") return WindowMode::Fullscreen;
    if (v == "borderless") return WindowMode::Borderless;
    return WindowMode::Windowed;
}
const char* WindowModeToString(WindowMode v) {
    switch (v) {
        case WindowMode::Fullscreen: return "Fullscreen";
        case WindowMode::Borderless: return "Borderless";
        case WindowMode::Windowed:   return "Windowed";
    }
    return "Windowed";
}

// ---------- defaults / load / save ----------

EngineConfig EngineConfig::Defaults() {
    EngineConfig c{};
    c.Window.Title         = "Freely Engine";
    c.Window.Width         = 1280;
    c.Window.Height        = 720;
    c.Window.Mode          = WindowMode::Windowed;
    c.Window.Resizable     = true;
    c.Window.VSync         = true;
    c.Window.MSAA          = 4;

    c.Renderer.API                     = RenderAPI::OpenGL;
    c.Renderer.EnableHDR               = true;
    c.Renderer.EnableShadows           = true;
    c.Renderer.ShadowMapResolution     = 2048;
    c.Renderer.MaxAnisotropy           = 16.0f;
    c.Renderer.EnableValidationLayers  = false;

    c.Physics.Backend3D    = PhysicsBackend3D::AsterCore;
    c.Physics.Backend2D    = PhysicsBackend2D::Box2D;
    c.Physics.FixedTimeStep= 1.0f / 60.0f;
    c.Physics.MaxSubSteps  = 4;
    c.Physics.Gravity3DX   = 0.0f;
    c.Physics.Gravity3DY   = -9.81f;
    c.Physics.Gravity3DZ   = 0.0f;
    c.Physics.Gravity2DX   = 0.0f;
    c.Physics.Gravity2DY   = -9.81f;
    c.Physics.WorkerThreads= 0; // auto

    c.Assets.AssetRoot         = "Assets";
    c.Assets.ShaderRoot        = "Assets/Shaders";
    c.Assets.PluginRoot        = "Plugins";
    c.Assets.CacheRoot         = "Cache";
    c.Assets.StreamingBudgetMB = 512;
    c.Assets.AsyncLoading      = true;

    c.Logging.EngineLevel      = LogLevel::Info;
    c.Logging.AppLevel         = LogLevel::Info;
    c.Logging.LogFile          = "Logs/freely.log";
    c.Logging.LogToFile        = true;
    c.Logging.LogToConsole     = true;
    c.Logging.ColorizedOutput  = true;

    c.Editor.ProjectRoot       = "Projects";
    c.Editor.LayoutFile        = "Editor/Layout.ini";
    c.Editor.WindowRounding    = 6.0f;
    c.Editor.FrameRounding     = 4.0f;
    c.Editor.GrabRounding      = 4.0f;
    c.Editor.Theme             = "Dark";
    c.Editor.FontPath          = "Assets/Fonts/Roboto-Regular.ttf";
    c.Editor.FontSize          = 16.0f;
    c.Editor.EnableDocking     = true;
    c.Editor.EnableViewports   = true;
    return c;
}

EngineConfig EngineConfig::LoadFromFile(const std::filesystem::path& path) {
    EngineConfig c = Defaults();

    std::ifstream in(path);
    if (!in) {
        return c;
    }

    FlatConfig f = FlatConfig::Parse(in);

    // [window]
    c.Window.Title     = StripQuotes(f.Get("window", "title",     c.Window.Title));
    c.Window.Width     = ParseInt   (f.Get("window", "width"),     c.Window.Width);
    c.Window.Height    = ParseInt   (f.Get("window", "height"),    c.Window.Height);
    c.Window.Mode      = WindowModeFromString(f.Get("window", "mode", WindowModeToString(c.Window.Mode)));
    c.Window.Resizable = ParseBool  (f.Get("window", "resizable"), c.Window.Resizable);
    c.Window.VSync     = ParseBool  (f.Get("window", "vsync"),     c.Window.VSync);
    c.Window.MSAA      = ParseInt   (f.Get("window", "msaa"),      c.Window.MSAA);

    // [renderer]
    c.Renderer.API                    = RenderAPIFromString(f.Get("renderer", "api", RenderAPIToString(c.Renderer.API)));
    c.Renderer.EnableHDR              = ParseBool (f.Get("renderer", "hdr"),                c.Renderer.EnableHDR);
    c.Renderer.EnableShadows          = ParseBool (f.Get("renderer", "shadows"),            c.Renderer.EnableShadows);
    c.Renderer.ShadowMapResolution    = ParseInt  (f.Get("renderer", "shadow_map_size"),    c.Renderer.ShadowMapResolution);
    c.Renderer.MaxAnisotropy          = ParseFloat(f.Get("renderer", "max_anisotropy"),     c.Renderer.MaxAnisotropy);
    c.Renderer.EnableValidationLayers = ParseBool (f.Get("renderer", "validation_layers"),  c.Renderer.EnableValidationLayers);

    // [physics]
    c.Physics.Backend3D     = PhysicsBackend3DFromString(f.Get("physics", "backend_3d", PhysicsBackend3DToString(c.Physics.Backend3D)));
    c.Physics.Backend2D     = PhysicsBackend2DFromString(f.Get("physics", "backend_2d", PhysicsBackend2DToString(c.Physics.Backend2D)));
    c.Physics.FixedTimeStep = ParseFloat(f.Get("physics", "fixed_timestep"),     c.Physics.FixedTimeStep);
    c.Physics.MaxSubSteps   = ParseInt  (f.Get("physics", "max_substeps"),       c.Physics.MaxSubSteps);
    c.Physics.Gravity3DX    = ParseFloat(f.Get("physics", "gravity_3d_x"),       c.Physics.Gravity3DX);
    c.Physics.Gravity3DY    = ParseFloat(f.Get("physics", "gravity_3d_y"),       c.Physics.Gravity3DY);
    c.Physics.Gravity3DZ    = ParseFloat(f.Get("physics", "gravity_3d_z"),       c.Physics.Gravity3DZ);
    c.Physics.Gravity2DX    = ParseFloat(f.Get("physics", "gravity_2d_x"),       c.Physics.Gravity2DX);
    c.Physics.Gravity2DY    = ParseFloat(f.Get("physics", "gravity_2d_y"),       c.Physics.Gravity2DY);
    c.Physics.WorkerThreads = ParseInt  (f.Get("physics", "worker_threads"),     c.Physics.WorkerThreads);

    // [assets]
    c.Assets.AssetRoot         = StripQuotes(f.Get("assets", "asset_root",         c.Assets.AssetRoot));
    c.Assets.ShaderRoot        = StripQuotes(f.Get("assets", "shader_root",        c.Assets.ShaderRoot));
    c.Assets.PluginRoot        = StripQuotes(f.Get("assets", "plugin_root",        c.Assets.PluginRoot));
    c.Assets.CacheRoot         = StripQuotes(f.Get("assets", "cache_root",         c.Assets.CacheRoot));
    c.Assets.StreamingBudgetMB = ParseInt   (f.Get("assets", "streaming_budget_mb"),c.Assets.StreamingBudgetMB);
    c.Assets.AsyncLoading      = ParseBool  (f.Get("assets", "async_loading"),     c.Assets.AsyncLoading);

    // [logging]
    c.Logging.EngineLevel     = LogLevelFromString(f.Get("logging", "engine_level", LogLevelToString(c.Logging.EngineLevel)));
    c.Logging.AppLevel        = LogLevelFromString(f.Get("logging", "app_level",    LogLevelToString(c.Logging.AppLevel)));
    c.Logging.LogFile         = StripQuotes(f.Get("logging", "log_file",       c.Logging.LogFile));
    c.Logging.LogToFile       = ParseBool  (f.Get("logging", "log_to_file"),   c.Logging.LogToFile);
    c.Logging.LogToConsole    = ParseBool  (f.Get("logging", "log_to_console"),c.Logging.LogToConsole);
    c.Logging.ColorizedOutput = ParseBool  (f.Get("logging", "colorized"),     c.Logging.ColorizedOutput);

    // [editor]
    c.Editor.ProjectRoot     = StripQuotes(f.Get("editor", "project_root",     c.Editor.ProjectRoot));
    c.Editor.LayoutFile      = StripQuotes(f.Get("editor", "layout_file",      c.Editor.LayoutFile));
    c.Editor.WindowRounding  = ParseFloat (f.Get("editor", "window_rounding"), c.Editor.WindowRounding);
    c.Editor.FrameRounding   = ParseFloat (f.Get("editor", "frame_rounding"),  c.Editor.FrameRounding);
    c.Editor.GrabRounding    = ParseFloat (f.Get("editor", "grab_rounding"),   c.Editor.GrabRounding);
    c.Editor.Theme           = StripQuotes(f.Get("editor", "theme",            c.Editor.Theme));
    c.Editor.FontPath        = StripQuotes(f.Get("editor", "font_path",        c.Editor.FontPath));
    c.Editor.FontSize        = ParseFloat (f.Get("editor", "font_size"),       c.Editor.FontSize);
    c.Editor.EnableDocking   = ParseBool  (f.Get("editor", "enable_docking"),  c.Editor.EnableDocking);
    c.Editor.EnableViewports = ParseBool  (f.Get("editor", "enable_viewports"),c.Editor.EnableViewports);

    return c;
}

bool EngineConfig::SaveToFile(const std::filesystem::path& path) const {
    std::filesystem::create_directories(path.parent_path());
    std::ofstream out(path, std::ios::trunc);
    if (!out) return false;

    out << "# Freely Engine Configuration\n\n";

    out << "[window]\n";
    out << "title       = \"" << Window.Title << "\"\n";
    out << "width       = " << Window.Width << "\n";
    out << "height      = " << Window.Height << "\n";
    out << "mode        = \"" << WindowModeToString(Window.Mode) << "\"\n";
    out << "resizable   = " << (Window.Resizable ? "true" : "false") << "\n";
    out << "vsync       = " << (Window.VSync ? "true" : "false") << "\n";
    out << "msaa        = " << Window.MSAA << "\n\n";

    out << "[renderer]\n";
    out << "api                = \"" << RenderAPIToString(Renderer.API) << "\"\n";
    out << "hdr                = " << (Renderer.EnableHDR ? "true" : "false") << "\n";
    out << "shadows            = " << (Renderer.EnableShadows ? "true" : "false") << "\n";
    out << "shadow_map_size    = " << Renderer.ShadowMapResolution << "\n";
    out << "max_anisotropy     = " << Renderer.MaxAnisotropy << "\n";
    out << "validation_layers  = " << (Renderer.EnableValidationLayers ? "true" : "false") << "\n\n";

    out << "[physics]\n";
    out << "backend_3d     = \"" << PhysicsBackend3DToString(Physics.Backend3D) << "\"\n";
    out << "backend_2d     = \"" << PhysicsBackend2DToString(Physics.Backend2D) << "\"\n";
    out << "fixed_timestep = " << Physics.FixedTimeStep << "\n";
    out << "max_substeps   = " << Physics.MaxSubSteps << "\n";
    out << "gravity_3d_x   = " << Physics.Gravity3DX << "\n";
    out << "gravity_3d_y   = " << Physics.Gravity3DY << "\n";
    out << "gravity_3d_z   = " << Physics.Gravity3DZ << "\n";
    out << "gravity_2d_x   = " << Physics.Gravity2DX << "\n";
    out << "gravity_2d_y   = " << Physics.Gravity2DY << "\n";
    out << "worker_threads = " << Physics.WorkerThreads << "\n\n";

    out << "[assets]\n";
    out << "asset_root          = \"" << Assets.AssetRoot << "\"\n";
    out << "shader_root         = \"" << Assets.ShaderRoot << "\"\n";
    out << "plugin_root         = \"" << Assets.PluginRoot << "\"\n";
    out << "cache_root          = \"" << Assets.CacheRoot << "\"\n";
    out << "streaming_budget_mb = " << Assets.StreamingBudgetMB << "\n";
    out << "async_loading       = " << (Assets.AsyncLoading ? "true" : "false") << "\n\n";

    out << "[logging]\n";
    out << "engine_level   = \"" << LogLevelToString(Logging.EngineLevel) << "\"\n";
    out << "app_level      = \"" << LogLevelToString(Logging.AppLevel)    << "\"\n";
    out << "log_file       = \"" << Logging.LogFile << "\"\n";
    out << "log_to_file    = " << (Logging.LogToFile ? "true" : "false") << "\n";
    out << "log_to_console = " << (Logging.LogToConsole ? "true" : "false") << "\n";
    out << "colorized      = " << (Logging.ColorizedOutput ? "true" : "false") << "\n\n";

    out << "[editor]\n";
    out << "project_root     = \"" << Editor.ProjectRoot << "\"\n";
    out << "layout_file      = \"" << Editor.LayoutFile << "\"\n";
    out << "window_rounding  = " << Editor.WindowRounding << "\n";
    out << "frame_rounding   = " << Editor.FrameRounding << "\n";
    out << "grab_rounding    = " << Editor.GrabRounding << "\n";
    out << "theme            = \"" << Editor.Theme << "\"\n";
    out << "font_path        = \"" << Editor.FontPath << "\"\n";
    out << "font_size        = " << Editor.FontSize << "\n";
    out << "enable_docking   = " << (Editor.EnableDocking ? "true" : "false") << "\n";
    out << "enable_viewports = " << (Editor.EnableViewports ? "true" : "false") << "\n";

    return true;
}

std::filesystem::path EngineConfig::ResolveAsset(const std::string& relative) const {
    return std::filesystem::path(Assets.AssetRoot) / relative;
}

} // namespace Freely
