#include "VulkanSwapchain.h"
#include "Freely/Core/Logger.h"
#include <algorithm>

namespace Freely::RHI {

VulkanSwapchain::VulkanSwapchain(VulkanContext& context)
    : m_Context(context) {
}

VulkanSwapchain::~VulkanSwapchain() {
    Shutdown();
}

void VulkanSwapchain::Initialize(uint32_t width, uint32_t height) {
    CreateSwapchain(width, height);
    CreateImageViews();
}

void VulkanSwapchain::Recreate(uint32_t width, uint32_t height) {
    vkDeviceWaitIdle(m_Context.GetDevice());
    Cleanup();
    CreateSwapchain(width, height);
    CreateImageViews();
}

void VulkanSwapchain::Shutdown() {
    Cleanup();
}

void VulkanSwapchain::Cleanup() {
    for (auto imageView : m_ImageViews) {
        vkDestroyImageView(m_Context.GetDevice(), imageView, nullptr);
    }
    m_ImageViews.clear();
    
    if (m_Swapchain != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(m_Context.GetDevice(), m_Swapchain, nullptr);
        m_Swapchain = VK_NULL_HANDLE;
    }
}

void VulkanSwapchain::CreateSwapchain(uint32_t width, uint32_t height) {
    VkPhysicalDevice physicalDevice = m_Context.GetPhysicalDevice();
    VkSurfaceKHR surface = m_Context.GetSurface();

    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
    std::vector<VkSurfaceFormatKHR> formats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, formats.data());

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);
    std::vector<VkPresentModeKHR> presentModes(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, presentModes.data());

    // Choose format
    VkSurfaceFormatKHR surfaceFormat = formats[0];
    for (const auto& availableFormat : formats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            surfaceFormat = availableFormat;
            break;
        }
    }

    // Choose present mode
    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR; // Guaranteed
    for (const auto& availablePresentMode : presentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            presentMode = availablePresentMode;
            break;
        }
    }

    // Choose extent
    VkExtent2D extent;
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        extent = capabilities.currentExtent;
    } else {
        extent = {width, height};
        extent.width = std::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        extent.height = std::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
    }

    uint32_t imageCount = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
        imageCount = capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    uint32_t queueFamilyIndices[] = {m_Context.GetGraphicsQueueFamily(), m_Context.GetPresentQueueFamily()};
    if (m_Context.GetGraphicsQueueFamily() != m_Context.GetPresentQueueFamily()) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    createInfo.preTransform = capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(m_Context.GetDevice(), &createInfo, nullptr, &m_Swapchain) != VK_SUCCESS) {
        FL_ENGINE_ERROR("Failed to create Vulkan swapchain!");
    }

    vkGetSwapchainImagesKHR(m_Context.GetDevice(), m_Swapchain, &imageCount, nullptr);
    m_Images.resize(imageCount);
    vkGetSwapchainImagesKHR(m_Context.GetDevice(), m_Swapchain, &imageCount, m_Images.data());

    m_ImageFormat = surfaceFormat.format;
    m_Extent = extent;
}

void VulkanSwapchain::CreateImageViews() {
    m_ImageViews.resize(m_Images.size());
    for (size_t i = 0; i < m_Images.size(); i++) {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = m_Images[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = m_ImageFormat;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(m_Context.GetDevice(), &createInfo, nullptr, &m_ImageViews[i]) != VK_SUCCESS) {
            FL_ENGINE_ERROR("Failed to create image views!");
        }
    }
}

VkResult VulkanSwapchain::AcquireNextImage(uint32_t* imageIndex, VkSemaphore presentCompleteSemaphore) {
    return vkAcquireNextImageKHR(m_Context.GetDevice(), m_Swapchain, std::numeric_limits<uint64_t>::max(), presentCompleteSemaphore, VK_NULL_HANDLE, imageIndex);
}

VkResult VulkanSwapchain::QueuePresent(VkQueue presentQueue, uint32_t imageIndex, VkSemaphore waitSemaphore) {
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &waitSemaphore;

    VkSwapchainKHR swapChains[] = {m_Swapchain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    return vkQueuePresentKHR(presentQueue, &presentInfo);
}

} // namespace Freely::RHI
