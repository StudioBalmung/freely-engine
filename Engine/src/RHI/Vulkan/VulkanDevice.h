#pragma once

#include "Freely/RHI/IRenderDevice.h"
#include "VulkanContext.h"
#include "VulkanSwapchain.h"
#include <memory>

namespace Freely::RHI {

class VulkanDevice : public IRenderDevice {
public:
    VulkanDevice();
    ~VulkanDevice() override;

    bool Initialize(GLFWwindow* window) override;
    void Shutdown() override;

    GraphicsAPI               GetAPI() const override { return GraphicsAPI::Vulkan; }
    const DeviceCapabilities& GetCapabilities() const override { return m_Caps; }

    void BeginFrame() override;
    void EndFrame() override;
    void Present() override;
    void WaitIdle() override;
    void Resize(uint32_t width, uint32_t height) override;

    void SetViewport(const Viewport& vp) override;
    void SetScissor(const Rect2D& rect) override;
    void SetClearColor(float r, float g, float b, float a) override;
    void Clear(bool color = true, bool depth = true, bool stencil = false) override;

    std::shared_ptr<IBuffer>      CreateBuffer(const BufferDesc& desc) override;
    std::shared_ptr<ITexture>     CreateTexture(const TextureDesc& desc) override;
    std::shared_ptr<IShader>      CreateShader(const ShaderDesc& desc) override;
    std::shared_ptr<IPipeline>    CreatePipeline(const PipelineDesc& desc) override;
    std::shared_ptr<IFramebuffer> CreateFramebuffer(const FramebufferDesc& desc) override;

    void BindPipeline(const std::shared_ptr<IPipeline>& pipeline) override;
    void BindVertexBuffer(const std::shared_ptr<IBuffer>& buffer, uint32_t binding = 0) override;
    void BindIndexBuffer(const std::shared_ptr<IBuffer>& buffer, IndexType type) override;
    void BindTexture(const std::shared_ptr<ITexture>& texture, uint32_t slot) override;
    void BindUniformBuffer(const std::shared_ptr<IBuffer>& buffer, uint32_t binding) override;
    void BindFramebuffer(const std::shared_ptr<IFramebuffer>& framebuffer) override;
    void UnbindFramebuffer() override;

    void Draw(uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t firstVertex = 0) override;
    void DrawIndexed(uint32_t indexCount, uint32_t instanceCount = 1, uint32_t firstIndex = 0) override;
    void Dispatch(uint32_t groupX, uint32_t groupY, uint32_t groupZ) override;

    VulkanContext& GetContext() { return *m_Context; }

private:
    void CreateSyncObjects();
    void CreateCommandBuffers();

    std::unique_ptr<VulkanContext> m_Context;
    std::unique_ptr<VulkanSwapchain> m_Swapchain;

    DeviceCapabilities m_Caps{};
    GLFWwindow* m_Window = nullptr;

    static const int MAX_FRAMES_IN_FLIGHT = 2;
    uint32_t m_CurrentFrame = 0;
    uint32_t m_ImageIndex = 0;

    std::vector<VkSemaphore> m_ImageAvailableSemaphores;
    std::vector<VkSemaphore> m_RenderFinishedSemaphores;
    std::vector<VkFence> m_InFlightFences;

    VkCommandPool m_CommandPool = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> m_CommandBuffers;

    bool m_FramebufferResized = false;
    float m_ClearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
};

} // namespace Freely::RHI
