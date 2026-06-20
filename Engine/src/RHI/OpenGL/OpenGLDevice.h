#pragma once

#include "Freely/RHI/IRenderDevice.h"

#include <unordered_map>

struct GLFWwindow;

namespace Freely::RHI {

class OpenGLDevice : public IRenderDevice {
public:
    OpenGLDevice();
    ~OpenGLDevice() override;

    bool Initialize(GLFWwindow* window) override;
    void Shutdown() override;

    GraphicsAPI               GetAPI() const override { return GraphicsAPI::OpenGL; }
    const DeviceCapabilities& GetCapabilities() const override { return m_Caps; }

    void BeginFrame() override;
    void EndFrame() override;
    void Present() override;
    void WaitIdle() override;
    void Resize(uint32_t width, uint32_t height) override;

    void SetViewport(const Viewport& vp) override;
    void SetScissor(const Rect2D& rect) override;
    void SetClearColor(float r, float g, float b, float a) override;
    void Clear(bool color, bool depth, bool stencil) override;

    std::shared_ptr<IBuffer>      CreateBuffer(const BufferDesc& desc) override;
    std::shared_ptr<ITexture>     CreateTexture(const TextureDesc& desc) override;
    std::shared_ptr<IShader>      CreateShader(const ShaderDesc& desc) override;
    std::shared_ptr<IPipeline>    CreatePipeline(const PipelineDesc& desc) override;
    std::shared_ptr<IFramebuffer> CreateFramebuffer(const FramebufferDesc& desc) override;

    void BindPipeline(const std::shared_ptr<IPipeline>& pipeline) override;
    void BindVertexBuffer(const std::shared_ptr<IBuffer>& buffer, uint32_t binding) override;
    void BindIndexBuffer(const std::shared_ptr<IBuffer>& buffer, IndexType type) override;
    void BindTexture(const std::shared_ptr<ITexture>& texture, uint32_t slot) override;
    void BindUniformBuffer(const std::shared_ptr<IBuffer>& buffer, uint32_t binding) override;
    void BindFramebuffer(const std::shared_ptr<IFramebuffer>& framebuffer) override;
    void UnbindFramebuffer() override;

    void Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex) override;
    void DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex) override;
    void Dispatch(uint32_t groupX, uint32_t groupY, uint32_t groupZ) override;

private:
    GLFWwindow*         m_Window = nullptr;
    DeviceCapabilities  m_Caps{};
    uint32_t            m_VAO = 0;
    IndexType           m_IndexType = IndexType::UInt32;
    uint32_t            m_BoundProgram = 0;
    float               m_ClearColor[4] = { 0, 0, 0, 1 };
    bool                m_Initialized = false;
};

} // namespace Freely::RHI
