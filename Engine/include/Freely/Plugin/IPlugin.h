#pragma once

#include <string>
#include <cstdint>

namespace Freely {
class Engine;
}

namespace Freely::Plugin {

struct PluginInfo {
    std::string Name;
    std::string Version;
    std::string Author;
    std::string Description;
    uint32_t    ApiVersion = 1;
};

/// Base interface every plugin DLL/.so must implement.
class IPlugin {
public:
    virtual ~IPlugin() = default;
    virtual const PluginInfo& GetInfo() const = 0;
    virtual bool OnLoad(Freely::Engine& engine)   = 0;
    virtual void OnUnload(Freely::Engine& engine) = 0;
    virtual void OnUpdate(float deltaTime) {}
};

} // namespace Freely::Plugin

// Each plugin shared library exports `CreateFreelyPlugin` / `DestroyFreelyPlugin`.
#if defined(_WIN32)
    #define FREELY_PLUGIN_EXPORT extern "C" __declspec(dllexport)
#else
    #define FREELY_PLUGIN_EXPORT extern "C" __attribute__((visibility("default")))
#endif

using FreelyCreatePluginFn  = Freely::Plugin::IPlugin*(*)();
using FreelyDestroyPluginFn = void(*)(Freely::Plugin::IPlugin*);

#define FREELY_DEFINE_PLUGIN(ClassName)                                            \
    FREELY_PLUGIN_EXPORT Freely::Plugin::IPlugin* CreateFreelyPlugin() {           \
        return new ClassName();                                                    \
    }                                                                              \
    FREELY_PLUGIN_EXPORT void DestroyFreelyPlugin(Freely::Plugin::IPlugin* p) {    \
        delete p;                                                                  \
    }
