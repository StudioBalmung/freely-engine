#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <string>

namespace Freely::RHI {

class VulkanContext {
public:
    VulkanContext();
    ~VulkanContext();

    bool Initialize(GLFWwindow* window);
    void Shutdown();

    VkInstance GetInstance() const { return m_Instance; }
    VkPhysicalDevice GetPhysicalDevice() const { return m_PhysicalDevice; }
    VkDevice GetDevice() const { return m_Device; }
    VkQueue GetGraphicsQueue() const { return m_GraphicsQueue; }
    VkQueue GetPresentQueue() const { return m_PresentQueue; }
    uint32_t GetGraphicsQueueFamily() const { return m_GraphicsQueueFamily; }
    uint32_t GetPresentQueueFamily() const { return m_PresentQueueFamily; }
    VkSurfaceKHR GetSurface() const { return m_Surface; }
    VmaAllocator GetAllocator() const { return m_Allocator; }

    GLFWwindow* GetWindow() const { return m_Window; }

private:
    void CreateInstance();
    void SetupDebugMessenger();
    void CreateSurface();
    void PickPhysicalDevice();
    void CreateLogicalDevice();
    void CreateAllocator();

    bool CheckValidationLayerSupport();
    std::vector<const char*> GetRequiredExtensions();
    bool IsDeviceSuitable(VkPhysicalDevice device);
    bool CheckDeviceExtensionSupport(VkPhysicalDevice device);

    GLFWwindow* m_Window = nullptr;
    VkInstance m_Instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT m_DebugMessenger = VK_NULL_HANDLE;
    VkSurfaceKHR m_Surface = VK_NULL_HANDLE;

    VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
    VkDevice m_Device = VK_NULL_HANDLE;

    uint32_t m_GraphicsQueueFamily = -1;
    uint32_t m_PresentQueueFamily = -1;
    VkQueue m_GraphicsQueue = VK_NULL_HANDLE;
    VkQueue m_PresentQueue = VK_NULL_HANDLE;

    VmaAllocator m_Allocator = VK_NULL_HANDLE;

    const std::vector<const char*> m_ValidationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };
    const std::vector<const char*> m_DeviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

#ifdef NDEBUG
    const bool m_EnableValidationLayers = false;
#else
    const bool m_EnableValidationLayers = true;
#endif
};

} // namespace Freely::RHI
