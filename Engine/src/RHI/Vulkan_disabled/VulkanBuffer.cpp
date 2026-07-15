#ifdef FREELY_RHI_VULKAN

#include "VulkanBuffer.h"
#include "Freely/Core/Logger.h"

namespace Freely::RHI {

VulkanBuffer::VulkanBuffer(VulkanDevice& device, const BufferDesc& desc)
    : m_Device(device), m_Size(desc.Size), m_Type(desc.Type), m_DebugName(desc.DebugName) {
    
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = desc.Size;
    
    switch (desc.Type) {
        case BufferType::Vertex: bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT; break;
        case BufferType::Index:  bufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT; break;
        case BufferType::Uniform: bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT; break;
        case BufferType::Storage: bufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT; break;
    }
    
    bufferInfo.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT; // Allow updates
    
    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
    
    if (desc.Usage == BufferUsage::Dynamic || desc.Usage == BufferUsage::Stream) {
        allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
    }

    if (vmaCreateBuffer(m_Device.GetContext().GetAllocator(), &bufferInfo, &allocInfo, &m_Buffer, &m_Allocation, nullptr) != VK_SUCCESS) {
        FL_ENGINE_ERROR("Failed to create Vulkan Buffer!");
    }

    if (desc.Data) {
        Update(desc.Data, desc.Size, 0);
    }
}

VulkanBuffer::~VulkanBuffer() {
    vmaDestroyBuffer(m_Device.GetContext().GetAllocator(), m_Buffer, m_Allocation);
}

void VulkanBuffer::Update(const void* data, uint64_t size, uint64_t offset) {
    void* mappedData;
    if (vmaMapMemory(m_Device.GetContext().GetAllocator(), m_Allocation, &mappedData) == VK_SUCCESS) {
        memcpy((uint8_t*)mappedData + offset, data, size);
        vmaUnmapMemory(m_Device.GetContext().GetAllocator(), m_Allocation);
    } else {
        // Here we should use a staging buffer for device local memory.
        // For simplicity in this iteration, we fallback/log.
        FL_ENGINE_WARN("Failed to map buffer memory directly. Staging buffer required for DEVICE_LOCAL updates.");
    }
}

} // namespace Freely::RHI

#endif // FREELY_RHI_VULKAN
