#pragma once
#include "Freely/RHI/IRenderDevice.h"
#include "VulkanDevice.h"

namespace Freely::RHI {
class VulkanPipeline : public IPipeline {
public:
    VulkanPipeline(VulkanDevice& device, const PipelineDesc& desc);
    ~VulkanPipeline() override;

    const std::string& GetDebugName() const override { return m_DebugName; }
    VkPipeline GetHandle() const { return m_Pipeline; }
    VkPipelineLayout GetLayout() const { return m_Layout; }

private:
    VulkanDevice& m_Device;
    VkPipeline m_Pipeline = VK_NULL_HANDLE;
    VkPipelineLayout m_Layout = VK_NULL_HANDLE;
    std::string m_DebugName;
};
} // namespace Freely::RHI
