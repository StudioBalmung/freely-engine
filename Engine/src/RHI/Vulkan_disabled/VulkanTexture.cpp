#ifdef FREELY_RHI_VULKAN
#include "VulkanTexture.h"
#include "Freely/Core/Logger.h"

namespace Freely::RHI {
VulkanTexture::VulkanTexture(VulkanDevice& device, const TextureDesc& desc)
    : m_Device(device), m_Width(desc.Width), m_Height(desc.Height), m_Format(desc.Format), m_DebugName(desc.DebugName) {
    // Basic implementation structure.
    FL_ENGINE_WARN("VulkanTexture full creation not implemented yet.");
}

VulkanTexture::~VulkanTexture() {
    if (m_Sampler != VK_NULL_HANDLE) vkDestroySampler(m_Device.GetContext().GetDevice(), m_Sampler, nullptr);
    if (m_ImageView != VK_NULL_HANDLE) vkDestroyImageView(m_Device.GetContext().GetDevice(), m_ImageView, nullptr);
    if (m_Image != VK_NULL_HANDLE) vmaDestroyImage(m_Device.GetContext().GetAllocator(), m_Image, m_Allocation);
}
} // namespace Freely::RHI
#endif // FREELY_RHI_VULKAN
