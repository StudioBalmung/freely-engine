#pragma once
#include "Freely/RHI/IRenderDevice.h"
#include "VulkanDevice.h"

namespace Freely::RHI {
class VulkanShader : public IShader {
public:
    VulkanShader(VulkanDevice& device, const ShaderDesc& desc);
    ~VulkanShader() override;

    ShaderStage GetStage() const override { return m_Stage; }
    const std::string& GetDebugName() const override { return m_DebugName; }

    VkShaderModule GetHandle() const { return m_Module; }

private:
    VulkanDevice& m_Device;
    VkShaderModule m_Module = VK_NULL_HANDLE;
    ShaderStage m_Stage;
    std::string m_DebugName;
};
} // namespace Freely::RHI
