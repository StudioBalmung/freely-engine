#pragma once

#include "Freely/RHI/IRenderDevice.h"
#include "VulkanDevice.h"
#include <vk_mem_alloc.h>

namespace Freely::RHI {

class VulkanBuffer : public IBuffer {
public:
    VulkanBuffer(VulkanDevice& device, const BufferDesc& desc);
    ~VulkanBuffer() override;

    void Update(const void* data, uint64_t size, uint64_t offset = 0) override;
    uint64_t GetSize() const override { return m_Size; }
    BufferType GetType() const override { return m_Type; }
    const std::string& GetDebugName() const override { return m_DebugName; }

    VkBuffer GetHandle() const { return m_Buffer; }

private:
    VulkanDevice& m_Device;
    VkBuffer m_Buffer = VK_NULL_HANDLE;
    VmaAllocation m_Allocation = VK_NULL_HANDLE;
    
    uint64_t m_Size = 0;
    BufferType m_Type;
    std::string m_DebugName;
};

} // namespace Freely::RHI
