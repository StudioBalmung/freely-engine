// DirectX 12 backend stub. Compiled only when FREELY_RHI_D3D12 is defined.
// Provides a placeholder implementation. Full backend implementation pending.
#ifdef FREELY_RHI_D3D12

#include "Freely/RHI/IRenderDevice.h"
#include "Freely/Core/Logger.h"

namespace Freely::RHI {

class D3D12Device : public IRenderDevice {
public:
    bool Initialize(GLFWwindow*) override {
        FL_ENGINE_WARN("DirectX12 backend is registered but not yet fully implemented.");
        m_Caps.DeviceName = "DirectX12 (stub)";
        return false;
    }
    void Shutdown() override {}

    GraphicsAPI               GetAPI() const override { return GraphicsAPI::DirectX12; }
    const DeviceCapabilities& GetCapabilities() const override { return m_Caps; }

    void BeginFrame() override {}
    void EndFrame() override {}
    void Present() override {}
    void WaitIdle() override {}
    void Resize(uint32_t, uint32_t) override {}

    void SetViewport(const Viewport&) override {}
    void SetScissor(const Rect2D&) override {}
    void SetClearColor(float, float, float, float) override {}
    void Clear(bool, bool, bool) override {}

    std::shared_ptr<IBuffer>      CreateBuffer(const BufferDesc&) override      { return nullptr; }
    std::shared_ptr<ITexture>     CreateTexture(const TextureDesc&) override    { return nullptr; }
    std::shared_ptr<IShader>      CreateShader(const ShaderDesc&) override      { return nullptr; }
    std::shared_ptr<IPipeline>    CreatePipeline(const PipelineDesc&) override  { return nullptr; }
    std::shared_ptr<IFramebuffer> CreateFramebuffer(const FramebufferDesc&) override { return nullptr; }

    void BindPipeline(const std::shared_ptr<IPipeline>&) override {}
    void BindVertexBuffer(const std::shared_ptr<IBuffer>&, uint32_t) override {}
    void BindIndexBuffer(const std::shared_ptr<IBuffer>&, IndexType) override {}
    void BindTexture(const std::shared_ptr<ITexture>&, uint32_t) override {}
    void BindUniformBuffer(const std::shared_ptr<IBuffer>&, uint32_t) override {}
    void BindFramebuffer(const std::shared_ptr<IFramebuffer>&) override {}
    void UnbindFramebuffer() override {}

    void Draw(uint32_t, uint32_t, uint32_t) override {}
    void DrawIndexed(uint32_t, uint32_t, uint32_t) override {}
    void Dispatch(uint32_t, uint32_t, uint32_t) override {}

private:
    DeviceCapabilities m_Caps{};
};

std::unique_ptr<IRenderDevice> CreateD3D12Device() {
    return std::make_unique<D3D12Device>();
}

} // namespace Freely::RHI

#endif // FREELY_RHI_D3D12
