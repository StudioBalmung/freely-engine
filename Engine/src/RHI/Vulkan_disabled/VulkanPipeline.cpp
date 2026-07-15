#ifdef FREELY_RHI_VULKAN
#include "VulkanPipeline.h"
#include "Freely/Core/Logger.h"

namespace Freely::RHI {
VulkanPipeline::VulkanPipeline(VulkanDevice& device, const PipelineDesc& desc)
    : m_Device(device), m_DebugName(desc.DebugName) {
    // Pipeline creation logic would go here.
    FL_ENGINE_WARN("VulkanPipeline full creation not implemented yet.");
}

VulkanPipeline::~VulkanPipeline() {
    if (m_Pipeline != VK_NULL_HANDLE) vkDestroyPipeline(m_Device.GetContext().GetDevice(), m_Pipeline, nullptr);
    if (m_Layout != VK_NULL_HANDLE) vkDestroyPipelineLayout(m_Device.GetContext().GetDevice(), m_Layout, nullptr);
}
} // namespace Freely::RHI
#endif // FREELY_RHI_VULKAN
