#ifdef FREELY_RHI_VULKAN
#include "VulkanShader.h"
#include "Freely/Core/Logger.h"
#include <shaderc/shaderc.hpp>

namespace Freely::RHI {

static shaderc_shader_kind GetShadercKind(ShaderStage stage) {
    switch (stage) {
        case ShaderStage::Vertex: return shaderc_vertex_shader;
        case ShaderStage::Fragment: return shaderc_fragment_shader;
        case ShaderStage::Compute: return shaderc_compute_shader;
        case ShaderStage::Geometry: return shaderc_geometry_shader;
        case ShaderStage::TessControl: return shaderc_tess_control_shader;
        case ShaderStage::TessEvaluation: return shaderc_tess_evaluation_shader;
        default: return shaderc_glsl_infer_from_source;
    }
}

VulkanShader::VulkanShader(VulkanDevice& device, const ShaderDesc& desc)
    : m_Device(device), m_Stage(desc.Stage), m_DebugName(desc.DebugName) {
    
    shaderc::Compiler compiler;
    shaderc::CompileOptions options;
    options.SetOptimizationLevel(shaderc_optimization_level_performance);
    
    shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(
        desc.Source, GetShadercKind(desc.Stage), desc.DebugName.c_str(), options);

    if (module.GetCompilationStatus() != shaderc_compilation_status_success) {
        FL_ENGINE_ERROR("Shader compilation error ({0}): {1}", desc.DebugName, module.GetErrorMessage());
        return;
    }

    std::vector<uint32_t> spv(module.cbegin(), module.cend());

    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = spv.size() * sizeof(uint32_t);
    createInfo.pCode = spv.data();

    if (vkCreateShaderModule(m_Device.GetContext().GetDevice(), &createInfo, nullptr, &m_Module) != VK_SUCCESS) {
        FL_ENGINE_ERROR("Failed to create shader module for {0}", desc.DebugName);
    }
}

VulkanShader::~VulkanShader() {
    if (m_Module != VK_NULL_HANDLE) {
        vkDestroyShaderModule(m_Device.GetContext().GetDevice(), m_Module, nullptr);
    }
}
} // namespace Freely::RHI
#endif // FREELY_RHI_VULKAN
