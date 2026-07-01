#pragma once

// Freely Engine 0.4.2 — Build Target Abstraction
// Abstract interface for cross-platform build targets.

#include <string>
#include <vector>
#include <functional>
#include <filesystem>
#include <cstdint>

namespace Freely {

// ─── Platform / Architecture enums ──────────────────────────────────────────
enum class TargetPlatform : uint8_t {
    Windows,
    Linux,
    Android,
    Web,        // Emscripten / WASM
    macOS,
    iOS
};

enum class TargetArchitecture : uint8_t {
    x86_64,
    ARM64,
    WASM,
    x86
};

enum class BuildConfiguration : uint8_t {
    Debug,
    Release,
    Shipping    // Release + stripped symbols, optimized assets
};

// ─── Build Progress ─────────────────────────────────────────────────────────
struct BuildProgress {
    float Percentage = 0.0f;          // 0.0 – 1.0
    std::string CurrentStep;          // e.g., "Compiling Scripts..."
    std::string LastMessage;          // Last log line
    bool        HasError = false;
    std::string ErrorMessage;

    using ProgressCallback = std::function<void(const BuildProgress&)>;
};

// ─── Build Config ───────────────────────────────────────────────────────────
struct BuildConfig {
    std::filesystem::path ProjectRoot;        // Absolute path to project root
    std::filesystem::path OutputDirectory;    // Where to put the built artifacts
    BuildConfiguration    Configuration = BuildConfiguration::Release;

    // Application metadata
    std::string AppName       = "MyGame";
    std::string AppVersion    = "1.0.0";
    std::string AppIdentifier = "com.freely.mygame";

    // Scripting
    bool   CompileScriptsAOT = false;         // Lua2CPP / IL2CPP
    std::string ScriptEntryPoint;

    // Asset cooking
    bool   CookAssets     = true;
    bool   PackAssets     = true;             // Bundle into .fpak
    int    TextureQuality = 2;                // 0=Low, 1=Medium, 2=High

    // Icon
    std::filesystem::path IconPath;

    // Extra CMake defines
    std::vector<std::string> ExtraCMakeDefines;
};

// ─── IBuildTarget ───────────────────────────────────────────────────────────
/// Abstract interface that each platform build target implements.
class IBuildTarget {
public:
    virtual ~IBuildTarget() = default;

    /// Human-readable name, e.g., "Windows x64"
    virtual const char* GetName() const = 0;

    /// Platform enum
    virtual TargetPlatform GetPlatform() const = 0;

    /// Architecture enum
    virtual TargetArchitecture GetArchitecture() const = 0;

    /// Check if the required toolchain is available on this machine.
    virtual bool IsToolchainAvailable() const = 0;

    /// Return a human-readable description of what's needed if toolchain is missing.
    virtual std::string GetToolchainRequirements() const = 0;

    /// Configure the build (validate paths, check dependencies).
    /// Returns false + sets progress.ErrorMessage on failure.
    virtual bool Configure(const BuildConfig& config, BuildProgress& progress) = 0;

    /// Execute the build.
    /// Should call progress callback periodically.
    virtual bool Build(const BuildConfig& config, BuildProgress& progress,
                       BuildProgress::ProgressCallback callback = nullptr) = 0;

    /// Package the build output (create installer, APK, zip, etc.).
    virtual bool Package(const BuildConfig& config, BuildProgress& progress,
                         BuildProgress::ProgressCallback callback = nullptr) = 0;

    /// Clean build artifacts.
    virtual void Clean(const BuildConfig& config) = 0;

    /// Get the expected output path after a successful build.
    virtual std::filesystem::path GetOutputPath(const BuildConfig& config) const = 0;
};

// ─── BuildTargetRegistry ────────────────────────────────────────────────────
/// Singleton registry of available build targets.
class BuildTargetRegistry {
public:
    static BuildTargetRegistry& Get() {
        static BuildTargetRegistry instance;
        return instance;
    }

    void Register(std::unique_ptr<IBuildTarget> target) {
        m_Targets.push_back(std::move(target));
    }

    const std::vector<std::unique_ptr<IBuildTarget>>& GetAll() const {
        return m_Targets;
    }

    IBuildTarget* FindByName(const std::string& name) const {
        for (auto& t : m_Targets) {
            if (t->GetName() == name) return t.get();
        }
        return nullptr;
    }

    IBuildTarget* FindByPlatform(TargetPlatform platform) const {
        for (auto& t : m_Targets) {
            if (t->GetPlatform() == platform) return t.get();
        }
        return nullptr;
    }

private:
    BuildTargetRegistry() = default;
    std::vector<std::unique_ptr<IBuildTarget>> m_Targets;
};

} // namespace Freely
