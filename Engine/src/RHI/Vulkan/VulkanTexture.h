#pragma once
#include "Freely/RHI/IRenderDevice.h"
#include "VulkanDevice.h"

namespace Freely::RHI {
class VulkanTexture : public ITexture {
public:
    VulkanTexture(VulkanDevice& device, const TextureDesc& desc);
    ~VulkanTexture() override;

    uint32_t GetWidth() const override { return m_Width; }
    uint32_t GetHeight() const override { return m_Height; }
    TextureFormat GetFormat() const override { return m_Format; }
    void* GetNativeHandle() const override { return (void*)m_ImageView; } // E.g., for ImGui descriptor
    const std::string& GetDebugName() const override { return m_DebugName; }

private:
    VulkanDevice& m_Device;
    VkImage m_Image = VK_NULL_HANDLE;
    VkImageView m_ImageView = VK_NULL_HANDLE;
    VkSampler m_Sampler = VK_NULL_HANDLE;
    VmaAllocation m_Allocation = VK_NULL_HANDLE;

    uint32_t m_Width = 0;
    uint32_t m_Height = 0;
    TextureFormat m_Format;
    std::string m_DebugName;
};
} // namespace Freely::RHI
