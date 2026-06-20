#include "Freely/Plugin/PluginManager.h"
#include "Freely/Core/Logger.h"

#if defined(_WIN32)
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    using LibHandle = HMODULE;
    static LibHandle FL_LoadLib(const wchar_t* p)               { return LoadLibraryW(p); }
    static void*     FL_GetSym (LibHandle h, const char* name)  { return reinterpret_cast<void*>(GetProcAddress(h, name)); }
    static void      FL_FreeLib(LibHandle h)                    { FreeLibrary(h); }
    static const wchar_t* FL_LibExt() { return L".dll"; }
#else
    #include <dlfcn.h>
    using LibHandle = void*;
    static LibHandle FL_LoadLib(const char* p)                  { return dlopen(p, RTLD_NOW | RTLD_LOCAL); }
    static void*     FL_GetSym (LibHandle h, const char* name)  { return dlsym(h, name); }
    static void      FL_FreeLib(LibHandle h)                    { dlclose(h); }
    static const char* FL_LibExt() {
    #if defined(__APPLE__)
        return ".dylib";
    #else
        return ".so";
    #endif
    }
#endif

namespace fs = std::filesystem;

namespace Freely::Plugin {

PluginManager::~PluginManager() {
    // Plugins should be explicitly unloaded by the Engine before destruction.
}

bool PluginManager::LoadPlugin(const fs::path& path, Engine& engine) {
#if defined(_WIN32)
    LibHandle lib = FL_LoadLib(path.wstring().c_str());
#else
    LibHandle lib = FL_LoadLib(path.string().c_str());
#endif
    if (!lib) {
        FL_ENGINE_ERROR("Failed to load plugin '{}'", path.string());
        return false;
    }

    auto createFn  = reinterpret_cast<FreelyCreatePluginFn> (FL_GetSym(lib, "CreateFreelyPlugin"));
    auto destroyFn = reinterpret_cast<FreelyDestroyPluginFn>(FL_GetSym(lib, "DestroyFreelyPlugin"));
    if (!createFn || !destroyFn) {
        FL_ENGINE_ERROR("Plugin '{}' missing required entrypoints.", path.string());
        FL_FreeLib(lib);
        return false;
    }

    IPlugin* instance = createFn();
    if (!instance) {
        FL_ENGINE_ERROR("Plugin '{}' CreateFreelyPlugin returned null.", path.string());
        FL_FreeLib(lib);
        return false;
    }

    if (!instance->OnLoad(engine)) {
        FL_ENGINE_ERROR("Plugin '{}' OnLoad returned false.", instance->GetInfo().Name);
        destroyFn(instance);
        FL_FreeLib(lib);
        return false;
    }

    LoadedPlugin lp;
    lp.Info          = instance->GetInfo();
    lp.LibraryHandle = lib;
    lp.Instance      = instance;
    lp.DestroyFn     = destroyFn;
    lp.Path          = path;
    m_Plugins.push_back(lp);

    FL_ENGINE_INFO("Loaded plugin: {} v{} by {}", lp.Info.Name, lp.Info.Version, lp.Info.Author);
    return true;
}

void PluginManager::DiscoverAndLoadAll(const fs::path& directory, Engine& engine) {
    if (!fs::exists(directory)) {
        FL_ENGINE_INFO("Plugin directory '{}' does not exist - skipping.", directory.string());
        return;
    }

#if defined(_WIN32)
    const std::wstring ext = FL_LibExt();
#else
    const std::string ext = FL_LibExt();
#endif

    for (const auto& entry : fs::directory_iterator(directory)) {
        if (!entry.is_regular_file()) continue;
#if defined(_WIN32)
        if (entry.path().extension().wstring() != ext) continue;
#else
        if (entry.path().extension().string() != ext) continue;
#endif
        LoadPlugin(entry.path(), engine);
    }
}

void PluginManager::UnloadAll(Engine& engine) {
    for (auto it = m_Plugins.rbegin(); it != m_Plugins.rend(); ++it) {
        if (it->Instance) {
            it->Instance->OnUnload(engine);
            if (it->DestroyFn) it->DestroyFn(it->Instance);
            it->Instance = nullptr;
        }
        if (it->LibraryHandle) {
            FL_FreeLib(static_cast<LibHandle>(it->LibraryHandle));
            it->LibraryHandle = nullptr;
        }
    }
    m_Plugins.clear();
}

void PluginManager::UpdateAll(float deltaTime) {
    for (auto& p : m_Plugins) {
        if (p.Instance) p.Instance->OnUpdate(deltaTime);
    }
}

} // namespace Freely::Plugin
