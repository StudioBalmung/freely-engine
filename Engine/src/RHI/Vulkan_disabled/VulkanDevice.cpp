#ifdef FREELY_RHI_VULKAN

#include "VulkanDevice.h"
#include "Freely/Core/Logger.h"

namespace Freely::RHI {

VulkanDevice::VulkanDevice() {
    m_Context = std::make_unique<VulkanContext>();
}

VulkanDevice::~VulkanDevice() {
    Shutdown();
}

bool VulkanDevice::Initialize(GLFWwindow* window) {
    m_Window = window;
    
    if (!m_Context->Initialize(window)) {
        return false;
    }

    m_Caps.DeviceName = "Vulkan Device";
    m_Caps.VendorName = "Vulkan Vendor";
    m_Caps.DriverVersion = "1.0.0";

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    m_Swapchain = std::make_unique<VulkanSwapchain>(*m_Context);
    m_Swapchain->Initialize(static_cast<uint32_t>(width), static_cast<uint32_t>(height));

    CreateSyncObjects();
    CreateCommandBuffers();

    FL_ENGINE_INFO("VulkanDevice successfully initialized.");
    return true;
}

void VulkanDevice::Shutdown() {
    WaitIdle();

    if (m_Context && m_Context->GetDevice() != VK_NULL_HANDLE) {
        VkDevice device = m_Context->GetDevice();

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroySemaphore(device, m_RenderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(device, m_ImageAvailableSemaphores[i], nullptr);
            vkDestroyFence(device, m_InFlightFences[i], nullptr);
        }

        vkDestroyCommandPool(device, m_CommandPool, nullptr);
    }

    m_Swapchain.reset();
    if (m_Context) {
        m_Context->Shutdown();
        m_Context.reset();
    }
}

void VulkanDevice::CreateSyncObjects() {
    m_ImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    m_RenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    m_InFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VkDevice device = m_Context->GetDevice();

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &m_ImageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(device, &semaphoreInfo, nullptr, &m_RenderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(device, &fenceInfo, nullptr, &m_InFlightFences[i]) != VK_SUCCESS) {
            FL_ENGINE_ERROR("Failed to create synchronization objects for a frame!");
        }
    }
}

void VulkanDevice::CreateCommandBuffers() {
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = m_Context->GetGraphicsQueueFamily();

    if (vkCreateCommandPool(m_Context->GetDevice(), &poolInfo, nullptr, &m_CommandPool) != VK_SUCCESS) {
        FL_ENGINE_ERROR("Failed to create command pool!");
    }

    m_CommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_CommandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)m_CommandBuffers.size();

    if (vkAllocateCommandBuffers(m_Context->GetDevice(), &allocInfo, m_CommandBuffers.data()) != VK_SUCCESS) {
        FL_ENGINE_ERROR("Failed to allocate command buffers!");
    }
}

void VulkanDevice::WaitIdle() {
    if (m_Context && m_Context->GetDevice() != VK_NULL_HANDLE) {
        vkDeviceWaitIdle(m_Context->GetDevice());
    }
}

void VulkanDevice::Resize(uint32_t width, uint32_t height) {
    m_FramebufferResized = true;
}

void VulkanDevice::BeginFrame() {
    vkWaitForFences(m_Context->GetDevice(), 1, &m_InFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX);

    VkResult result = m_Swapchain->AcquireNextImage(&m_ImageIndex, m_ImageAvailableSemaphores[m_CurrentFrame]);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        int width = 0, height = 0;
        glfwGetFramebufferSize(m_Window, &width, &height);
        while (width == 0 || height == 0) {
            glfwGetFramebufferSize(m_Window, &width, &height);
            glfwWaitEvents();
        }
        m_Swapchain->Recreate(width, height);
        return; // Skip this frame
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        FL_ENGINE_ERROR("Failed to acquire swap chain image!");
    }

    vkResetFences(m_Context->GetDevice(), 1, &m_InFlightFences[m_CurrentFrame]);

    vkResetCommandBuffer(m_CommandBuffers[m_CurrentFrame], 0);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(m_CommandBuffers[m_CurrentFrame], &beginInfo) != VK_SUCCESS) {
        FL_ENGINE_ERROR("Failed to begin recording command buffer!");
    }

    // Usually RenderPass begins here. For now, since IRenderDevice doesn't strictly dictate renderpass structure
    // at BeginFrame (unless we bind a default framebuffer), we just leave the command buffer open.
}

void VulkanDevice::EndFrame() {
    if (vkEndCommandBuffer(m_CommandBuffers[m_CurrentFrame]) != VK_SUCCESS) {
        FL_ENGINE_ERROR("Failed to record command buffer!");
    }
}

void VulkanDevice::Present() {
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {m_ImageAvailableSemaphores[m_CurrentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_CommandBuffers[m_CurrentFrame];

    VkSemaphore signalSemaphores[] = {m_RenderFinishedSemaphores[m_CurrentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(m_Context->GetGraphicsQueue(), 1, &submitInfo, m_InFlightFences[m_CurrentFrame]) != VK_SUCCESS) {
        FL_ENGINE_ERROR("Failed to submit draw command buffer!");
    }

    VkResult result = m_Swapchain->QueuePresent(m_Context->GetPresentQueue(), m_ImageIndex, m_RenderFinishedSemaphores[m_CurrentFrame]);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_FramebufferResized) {
        m_FramebufferResized = false;
        int width = 0, height = 0;
        glfwGetFramebufferSize(m_Window, &width, &height);
        while (width == 0 || height == 0) {
            glfwGetFramebufferSize(m_Window, &width, &height);
            glfwWaitEvents();
        }
        m_Swapchain->Recreate(width, height);
    } else if (result != VK_SUCCESS) {
        FL_ENGINE_ERROR("Failed to present swap chain image!");
    }

    m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void VulkanDevice::SetViewport(const Viewport& vp) {
    VkViewport viewport{};
    viewport.x = vp.X;
    viewport.y = vp.Y;
    viewport.width = vp.Width;
    viewport.height = vp.Height;
    viewport.minDepth = vp.MinDepth;
    viewport.maxDepth = vp.MaxDepth;
    vkCmdSetViewport(m_CommandBuffers[m_CurrentFrame], 0, 1, &viewport);
}

void VulkanDevice::SetScissor(const Rect2D& rect) {
    VkRect2D scissor{};
    scissor.offset = {rect.X, rect.Y};
    scissor.extent = {rect.Width, rect.Height};
    vkCmdSetScissor(m_CommandBuffers[m_CurrentFrame], 0, 1, &scissor);
}

void VulkanDevice::SetClearColor(float r, float g, float b, float a) {
    m_ClearColor[0] = r;
    m_ClearColor[1] = g;
    m_ClearColor[2] = b;
    m_ClearColor[3] = a;
}

void VulkanDevice::Clear(bool color, bool depth, bool stencil) {
    // In Vulkan, clear is usually done as part of the render pass begin using loadOp = CLEAR.
    // However, if we need explicit clear cmds:
    VkClearAttachment attachments[2] = {};
    uint32_t attachmentCount = 0;

    if (color) {
        attachments[attachmentCount].aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        attachments[attachmentCount].colorAttachment = 0;
        attachments[attachmentCount].clearValue.color = { {m_ClearColor[0], m_ClearColor[1], m_ClearColor[2], m_ClearColor[3]} };
        attachmentCount++;
    }
    
    // Simple placeholder for now.
}

std::shared_ptr<IBuffer> VulkanDevice::CreateBuffer(const BufferDesc& desc) { return nullptr; }
std::shared_ptr<ITexture> VulkanDevice::CreateTexture(const TextureDesc& desc) { return nullptr; }
std::shared_ptr<IShader> VulkanDevice::CreateShader(const ShaderDesc& desc) { return nullptr; }
std::shared_ptr<IPipeline> VulkanDevice::CreatePipeline(const PipelineDesc& desc) { return nullptr; }
std::shared_ptr<IFramebuffer> VulkanDevice::CreateFramebuffer(const FramebufferDesc& desc) { return nullptr; }

void VulkanDevice::BindPipeline(const std::shared_ptr<IPipeline>& pipeline) {}
void VulkanDevice::BindVertexBuffer(const std::shared_ptr<IBuffer>& buffer, uint32_t binding) {}
void VulkanDevice::BindIndexBuffer(const std::shared_ptr<IBuffer>& buffer, IndexType type) {}
void VulkanDevice::BindTexture(const std::shared_ptr<ITexture>& texture, uint32_t slot) {}
void VulkanDevice::BindUniformBuffer(const std::shared_ptr<IBuffer>& buffer, uint32_t binding) {}
void VulkanDevice::BindFramebuffer(const std::shared_ptr<IFramebuffer>& framebuffer) {}
void VulkanDevice::UnbindFramebuffer() {}

void VulkanDevice::Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex) {
    vkCmdDraw(m_CommandBuffers[m_CurrentFrame], vertexCount, instanceCount, firstVertex, 0);
}
void VulkanDevice::DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex) {
    vkCmdDrawIndexed(m_CommandBuffers[m_CurrentFrame], indexCount, instanceCount, firstIndex, 0, 0);
}
void VulkanDevice::Dispatch(uint32_t groupX, uint32_t groupY, uint32_t groupZ) {
    vkCmdDispatch(m_CommandBuffers[m_CurrentFrame], groupX, groupY, groupZ);
}

std::unique_ptr<IRenderDevice> CreateVulkanDevice() {
    return std::make_unique<VulkanDevice>();
}

} // namespace Freely::RHI

#endif // FREELY_RHI_VULKAN
