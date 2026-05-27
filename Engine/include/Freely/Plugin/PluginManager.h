#pragma once

#include "IPlugin.h"

#include <filesystem>
#include <memory>
#include <vector>
#include <string>

namespace Freely {
class Engine;
}

namespace Freely::Plugin {

struct LoadedPlugin {
    PluginInfo  Info;
    void*       LibraryHandle = nullptr;
    IPlugin*    Instance      = nullptr;
    FreelyDestroyPluginFn DestroyFn = nullptr;
    std::filesystem::path Path;
};

/// Manages discovery, loading, and unloading of plugins from a directory.
class PluginManager {
public:
    PluginManager() = default;
    ~PluginManager();

    /// Load every shared library in `directory` that exports CreateFreelyPlugin.
    void DiscoverAndLoadAll(const std::filesystem::path& directory, Engine& engine);

    /// Load a single plugin by path.
    bool LoadPlugin(const std::filesystem::path& path, Engine& engine);

    /// Unload all plugins (called automatically on destruction).
    void UnloadAll(Engine& engine);

    /// Update lifecycle hook (called every frame).
    void UpdateAll(float deltaTime);

    const std::vector<LoadedPlugin>& GetPlugins() const { return m_Plugins; }

private:
    std::vector<LoadedPlugin> m_Plugins;
};

} // namespace Freely::Plugin
