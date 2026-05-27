#pragma once

#include "RHITypes.h"
#include "RHIResources.h"

#include <memory>

struct GLFWwindow;

namespace Freely::RHI {

struct DeviceCapabilities {
    std::string DeviceName;
    std::string VendorName;
    std::string DriverVersion;
    uint32_t    MaxTextureSize = 0;
    uint32_t    MaxColorAttachments = 0;
    uint32_t    MaxAnisotropy = 1;
    bool        SupportsCompute = false;
    bool        SupportsRayTracing = false;
    bool        SupportsBindless = false;
};

/// Abstract render device. One concrete implementation per graphics API
/// (OpenGL, Vulkan, DirectX12). Created by RenderDeviceFactory.
class IRenderDevice {
public:
    virtual ~IRenderDevice() = default;

    virtual bool Initialize(GLFWwindow* window) = 0;
    virtual void Shutdown() = 0;

    virtual GraphicsAPI                 GetAPI() const = 0;
    virtual const DeviceCapabilities&   GetCapabilities() const = 0;

    // Frame lifecycle
    virtual void BeginFrame() = 0;
    virtual void EndFrame() = 0;
    virtual void Present() = 0;
    virtual void WaitIdle() = 0;
    virtual void Resize(uint32_t width, uint32_t height) = 0;

    // Render state
    virtual void SetViewport(const Viewport& vp) = 0;
    virtual void SetScissor(const Rect2D& rect) = 0;
    virtual void SetClearColor(float r, float g, float b, float a) = 0;
    virtual void Clear(bool color = true, bool depth = true, bool stencil = false) = 0;

    // Resource creation
    virtual std::shared_ptr<IBuffer>      CreateBuffer(const BufferDesc& desc) = 0;
    virtual std::shared_ptr<ITexture>     CreateTexture(const TextureDesc& desc) = 0;
    virtual std::shared_ptr<IShader>      CreateShader(const ShaderDesc& desc) = 0;
    virtual std::shared_ptr<IPipeline>    CreatePipeline(const PipelineDesc& desc) = 0;
    virtual std::shared_ptr<IFramebuffer> CreateFramebuffer(const FramebufferDesc& desc) = 0;

    // Drawing
    virtual void BindPipeline(const std::shared_ptr<IPipeline>& pipeline) = 0;
    virtual void BindVertexBuffer(const std::shared_ptr<IBuffer>& buffer, uint32_t binding = 0) = 0;
    virtual void BindIndexBuffer(const std::shared_ptr<IBuffer>& buffer, IndexType type) = 0;
    virtual void BindTexture(const std::shared_ptr<ITexture>& texture, uint32_t slot) = 0;
    virtual void BindUniformBuffer(const std::shared_ptr<IBuffer>& buffer, uint32_t binding) = 0;
    virtual void BindFramebuffer(const std::shared_ptr<IFramebuffer>& framebuffer) = 0;
    virtual void UnbindFramebuffer() = 0;

    virtual void Draw(uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t firstVertex = 0) = 0;
    virtual void DrawIndexed(uint32_t indexCount, uint32_t instanceCount = 1, uint32_t firstIndex = 0) = 0;
    virtual void Dispatch(uint32_t groupX, uint32_t groupY, uint32_t groupZ) = 0;
};

/// Factory for creating render devices for the requested API.
class RenderDeviceFactory {
public:
    static std::unique_ptr<IRenderDevice> Create(GraphicsAPI api);
    static GraphicsAPI ResolveAuto();
};

} // namespace Freely::RHI
