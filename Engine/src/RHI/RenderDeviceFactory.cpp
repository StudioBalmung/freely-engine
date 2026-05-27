#include "Freely/RHI/IRenderDevice.h"
#include "Freely/Core/Logger.h"

#include <memory>

namespace Freely::RHI {

// Forward declarations of concrete backends.
std::unique_ptr<IRenderDevice> CreateOpenGLDevice();
#ifdef FREELY_RHI_VULKAN
std::unique_ptr<IRenderDevice> CreateVulkanDevice();
#endif
#ifdef FREELY_RHI_D3D12
std::unique_ptr<IRenderDevice> CreateD3D12Device();
#endif

GraphicsAPI RenderDeviceFactory::ResolveAuto() {
    // Preference order. Vulkan/DX12 stubs are placeholders for now.
#ifdef FREELY_RHI_VULKAN
    return GraphicsAPI::Vulkan;
#elif defined(FREELY_RHI_D3D12)
    return GraphicsAPI::DirectX12;
#else
    return GraphicsAPI::OpenGL;
#endif
}

std::unique_ptr<IRenderDevice> RenderDeviceFactory::Create(GraphicsAPI api) {
    if (api == GraphicsAPI::None) {
        FL_ENGINE_ERROR("Cannot create RenderDevice with API=None.");
        return nullptr;
    }
    if (api == GraphicsAPI::Vulkan) {
#ifdef FREELY_RHI_VULKAN
        return CreateVulkanDevice();
#else
        FL_ENGINE_WARN("Vulkan backend not compiled in (FREELY_RHI_VULKAN). Falling back to OpenGL.");
        return CreateOpenGLDevice();
#endif
    }
    if (api == GraphicsAPI::DirectX12) {
#ifdef FREELY_RHI_D3D12
        return CreateD3D12Device();
#else
        FL_ENGINE_WARN("DirectX12 backend not compiled in (FREELY_RHI_D3D12). Falling back to OpenGL.");
        return CreateOpenGLDevice();
#endif
    }
    return CreateOpenGLDevice();
}

} // namespace Freely::RHI
