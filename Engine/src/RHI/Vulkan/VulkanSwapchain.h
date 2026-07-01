#pragma once

#include "VulkanContext.h"
#include <vector>

namespace Freely::RHI {

class VulkanSwapchain {
public:
    VulkanSwapchain(VulkanContext& context);
    ~VulkanSwapchain();

    void Initialize(uint32_t width, uint32_t height);
    void Recreate(uint32_t width, uint32_t height);
    void Shutdown();

    VkSwapchainKHR GetSwapchain() const { return m_Swapchain; }
    VkFormat GetImageFormat() const { return m_ImageFormat; }
    VkExtent2D GetExtent() const { return m_Extent; }
    const std::vector<VkImageView>& GetImageViews() const { return m_ImageViews; }
    
    VkResult AcquireNextImage(uint32_t* imageIndex, VkSemaphore presentCompleteSemaphore);
    VkResult QueuePresent(VkQueue presentQueue, uint32_t imageIndex, VkSemaphore waitSemaphore);

private:
    void CreateSwapchain(uint32_t width, uint32_t height);
    void CreateImageViews();
    void Cleanup();

    VulkanContext& m_Context;
    VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;
    std::vector<VkImage> m_Images;
    std::vector<VkImageView> m_ImageViews;
    VkFormat m_ImageFormat;
    VkExtent2D m_Extent;
};

} // namespace Freely::RHI
