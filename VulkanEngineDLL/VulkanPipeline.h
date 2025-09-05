#pragma once
#include "VulkanRenderer.h"
#include "JsonStruct.h"
#include "VulkanShader.h"

struct VulkanPipeline
{
    VkGuid RenderPipelineId;
    size_t DescriptorSetLayoutCount;
    size_t DescriptorSetCount;
    String PushConstantName;
    VkDescriptorPool DescriptorPool = VK_NULL_HANDLE;
    VkDescriptorSetLayout* DescriptorSetLayoutList = nullptr;
    VkDescriptorSet* DescriptorSetList = nullptr;
    VkPipeline Pipeline = VK_NULL_HANDLE;
    VkPipelineLayout PipelineLayout = VK_NULL_HANDLE;
    VkPipelineCache PipelineCache = VK_NULL_HANDLE;
};

#ifdef __cplusplus
extern "C" {
#endif
DLL_EXPORT VulkanPipeline VulkanPipeline_CreateRenderPipeline(VkDevice device, RenderPipelineLoader& renderPipelineLoader);
DLL_EXPORT VulkanPipeline VulkanPipeline_RebuildSwapChain(VkDevice device, RenderPipelineLoader& renderPipelineLoader);
DLL_EXPORT void VulkanPipeline_Destroy(VkDevice device, VulkanPipeline& vulkanPipelineDLL);
#ifdef __cplusplus
}
#endif

VkDescriptorPool Pipeline_CreatePipelineDescriptorPool(VkDevice device, RenderPipelineLoader& renderPipelineLoader);
Vector<VkDescriptorSetLayout> Pipeline_CreatePipelineDescriptorSetLayout(VkDevice device, RenderPipelineLoader& renderPipelineLoader);
Vector<VkDescriptorSet> Pipeline_AllocatePipelineDescriptorSets(VkDevice device, RenderPipelineLoader& renderPipelineLoader, const VkDescriptorPool& descriptorPool, VkDescriptorSetLayout* descriptorSetLayoutList, size_t descriptorSetLayoutCount);
void Pipeline_UpdatePipelineDescriptorSets(VkDevice device, RenderPipelineLoader& renderPipelineLoader, VkDescriptorSet* descriptorSetList, size_t descriptorSetCount);
VkPipelineLayout Pipeline_CreatePipelineLayout(VkDevice device, RenderPipelineLoader& renderPipelineLoader, VkDescriptorSetLayout* descriptorSetLayoutList, size_t descriptorSetLayoutCount);
VkPipeline Pipeline_CreatePipeline(VkDevice device, RenderPipelineLoader& renderPipelineLoader, VkPipelineCache pipelineCache, VkPipelineLayout pipelineLayout, VkDescriptorSet* descriptorSetList, size_t descriptorSetCount);
void Pipeline_PipelineBindingData(RenderPipelineLoader& renderPipelineLoader);

void DebugRenderPipelineLoader(const RenderPipelineLoader& model, const std::string& indent = "");
void DebugGPUIncludes(const GPUIncludes& includes, const std::string& indent);
void DebugShaderPipelineData(const ShaderPipelineData& data, const std::string& indent);
void DebugShaderList(const char** shaderList, size_t count, const std::string& indent);
void DebugDescriptorBindingsList(const ShaderDescriptorBinding* bindings, size_t count, const std::string& indent);
void DebugShaderStructList(const ShaderStruct* structs, size_t count, const std::string& indent);
void DebugShaderVariableList(const ShaderVariable* variables, size_t count, const std::string& listName, const std::string& indent);
void DebugPushConstantList(const ShaderPushConstant* pushConstants, size_t count, const std::string& indent);
void DebugVertexInputBindingList(const VkVertexInputBindingDescription* bindings, size_t count, const std::string& indent);
void DebugVertexInputAttributeList(const VkVertexInputAttributeDescription* attributes, size_t count, const std::string& indent);
void DebugDescriptorBufferInfoList(const VkDescriptorBufferInfo* infos, size_t count, const std::string& name, const std::string& indent);
void DebugDescriptorImageInfoList(const VkDescriptorImageInfo* infos, size_t count, const std::string& indent);
void DebugViewportList(const VkViewport* viewports, size_t count, const std::string& indent);
void DebugScissorList(const VkRect2D* scissors, size_t count, const std::string& indent);
void DebugPipelineColorBlendAttachmentStateList(const VkPipelineColorBlendAttachmentState* states, size_t count, const std::string& indent);
void DebugPipelineInputAssemblyState(const VkPipelineInputAssemblyStateCreateInfo& state, const std::string& indent);
void DebugPipelineRasterizationState(const VkPipelineRasterizationStateCreateInfo& state, const std::string& indent);
void DebugPipelineMultisampleState(const VkPipelineMultisampleStateCreateInfo& state, const std::string& indent);
void DebugPipelineDepthStencilState(const VkPipelineDepthStencilStateCreateInfo& state, const std::string& indent);
void DebugPipelineColorBlendState(const VkPipelineColorBlendStateCreateInfo& state, const std::string& indent);

class RenderPipelineDebugger {
public:
    static void DebugRenderPipelineLoader(const RenderPipelineLoader& model, const std::string& indent = "") {
        std::cout << indent << "=== RenderPipelineLoader ===" << std::endl;
        std::cout << indent << "PipelineId: " << model.PipelineId.ToString() << std::endl;
        std::cout << indent << "RenderPassId: " << model.RenderPassId.ToString() << std::endl;
        std::cout << indent << "RenderPass: 0x" << std::hex << std::setw(16) << std::setfill('0') << (uint64_t)model.RenderPass << std::dec << std::endl;
        std::cout << indent << "RenderPassResolution: (" << model.RenderPassResolution.x << ", " << model.RenderPassResolution.y << ")" << std::endl;

        std::cout << indent << "--- GPUIncludes ---" << std::endl;
        DebugGPUIncludes(model.gpuIncludes, indent + "  ");

        std::cout << indent << "--- ShaderPipelineData ---" << std::endl;
        DebugShaderPipelineData(model.ShaderPiplineInfo, indent + "  ");

        std::cout << indent << "ViewportCount: " << model.ViewportCount << std::endl;
        DebugViewportList(model.ViewportList, model.ViewportCount, indent + "  ");

        std::cout << indent << "ScissorCount: " << model.ScissorCount << std::endl;
        DebugScissorList(model.ScissorList, model.ScissorCount, indent + "  ");

        std::cout << indent << "PipelineColorBlendAttachmentStateCount: " << model.PipelineColorBlendAttachmentStateCount << std::endl;
        DebugPipelineColorBlendAttachmentStateList(model.PipelineColorBlendAttachmentStateList, model.PipelineColorBlendAttachmentStateCount, indent + "  ");

        std::cout << indent << "PipelineInputAssemblyStateCreateInfo:" << std::endl;
        DebugPipelineInputAssemblyState(model.PipelineInputAssemblyStateCreateInfo, indent + "  ");

        std::cout << indent << "PipelineRasterizationStateCreateInfo:" << std::endl;
        DebugPipelineRasterizationState(model.PipelineRasterizationStateCreateInfo, indent + "  ");

        std::cout << indent << "PipelineMultisampleStateCreateInfo:" << std::endl;
        DebugPipelineMultisampleState(model.PipelineMultisampleStateCreateInfo, indent + "  ");

        std::cout << indent << "PipelineDepthStencilStateCreateInfo:" << std::endl;
        DebugPipelineDepthStencilState(model.PipelineDepthStencilStateCreateInfo, indent + "  ");

        std::cout << indent << "PipelineColorBlendStateCreateInfo:" << std::endl;
        DebugPipelineColorBlendState(model.PipelineColorBlendStateCreateInfoModel, indent + "  ");
    }

private:
    // Helper functions to map Vulkan and custom enums to string names
    static std::string GetVkDescriptorTypeString(VkDescriptorType type) {
        switch (type) {
        case VK_DESCRIPTOR_TYPE_SAMPLER: return "VK_DESCRIPTOR_TYPE_SAMPLER";
        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER: return "VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER";
        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE: return "VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE";
        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE: return "VK_DESCRIPTOR_TYPE_STORAGE_IMAGE";
        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER: return "VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER";
        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER: return "VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER";
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER: return "VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER";
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER: return "VK_DESCRIPTOR_TYPE_STORAGE_BUFFER";
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC: return "VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC";
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC: return "VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC";
        case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT: return "VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT";
        default: return "Unknown(" + std::to_string(type) + ")";
        }
    }

    static std::string GetVkShaderStageFlagsString(VkShaderStageFlags flags) {
        if (flags == 0) return "None";
        std::string result;
        if (flags & VK_SHADER_STAGE_VERTEX_BIT) result += "VK_SHADER_STAGE_VERTEX_BIT | ";
        if (flags & VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT) result += "VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT | ";
        if (flags & VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT) result += "VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT | ";
        if (flags & VK_SHADER_STAGE_GEOMETRY_BIT) result += "VK_SHADER_STAGE_GEOMETRY_BIT | ";
        if (flags & VK_SHADER_STAGE_FRAGMENT_BIT) result += "VK_SHADER_STAGE_FRAGMENT_BIT | ";
        if (flags & VK_SHADER_STAGE_COMPUTE_BIT) result += "VK_SHADER_STAGE_COMPUTE_BIT | ";
        if (flags & VK_SHADER_STAGE_ALL_GRAPHICS) result += "VK_SHADER_STAGE_ALL_GRAPHICS | ";
        if (flags & VK_SHADER_STAGE_ALL) result += "VK_SHADER_STAGE_ALL | ";
        if (!result.empty()) result = result.substr(0, result.size() - 3); // Remove trailing " | "
        return result.empty() ? "Unknown(" + std::to_string(flags) + ")" : result;
    }

    static std::string GetShaderMemberTypeString(ShaderMemberType type) {
        // Placeholder: Replace with actual ShaderMemberType enum values
        switch (type) {
        case shaderUnknown: return "shaderUnknown";
            // Add other cases based on your ShaderMemberType definition, e.g.:
            // case shaderFloat: return "shaderFloat";
            // case shaderInt: return "shaderInt";
        default: return "Unknown(" + std::to_string(type) + ")";
        }
    }

    static std::string GetDescriptorBindingPropertiesEnumString(DescriptorBindingPropertiesEnum type) {
        // Placeholder: Replace with actual DescriptorBindingPropertiesEnum values
        switch (type) {
        case 0: return "UnknownBindingType"; // Replace with actual enum name
            // Add other cases based on your DescriptorBindingPropertiesEnum definition
        default: return "Unknown(" + std::to_string(type) + ")";
        }
    }

    static std::string GetVkPrimitiveTopologyString(VkPrimitiveTopology topology) {
        switch (topology) {
        case VK_PRIMITIVE_TOPOLOGY_POINT_LIST: return "VK_PRIMITIVE_TOPOLOGY_POINT_LIST";
        case VK_PRIMITIVE_TOPOLOGY_LINE_LIST: return "VK_PRIMITIVE_TOPOLOGY_LINE_LIST";
        case VK_PRIMITIVE_TOPOLOGY_LINE_STRIP: return "VK_PRIMITIVE_TOPOLOGY_LINE_STRIP";
        case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST: return "VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST";
        case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP: return "VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP";
        default: return "Unknown(" + std::to_string(topology) + ")";
        }
    }

    static std::string GetVkPolygonModeString(VkPolygonMode mode) {
        switch (mode) {
        case VK_POLYGON_MODE_FILL: return "VK_POLYGON_MODE_FILL";
        case VK_POLYGON_MODE_LINE: return "VK_POLYGON_MODE_LINE";
        case VK_POLYGON_MODE_POINT: return "VK_POLYGON_MODE_POINT";
        default: return "Unknown(" + std::to_string(mode) + ")";
        }
    }

    static std::string GetVkCullModeFlagsString(VkCullModeFlags mode) {
        if (mode == 0) return "VK_CULL_MODE_NONE";
        std::string result;
        if (mode & VK_CULL_MODE_FRONT_BIT) result += "VK_CULL_MODE_FRONT_BIT | ";
        if (mode & VK_CULL_MODE_BACK_BIT) result += "VK_CULL_MODE_BACK_BIT | ";
        if (!result.empty()) result = result.substr(0, result.size() - 3);
        return result.empty() ? "Unknown(" + std::to_string(mode) + ")" : result;
    }

    static std::string GetVkFrontFaceString(VkFrontFace face) {
        switch (face) {
        case VK_FRONT_FACE_COUNTER_CLOCKWISE: return "VK_FRONT_FACE_COUNTER_CLOCKWISE";
        case VK_FRONT_FACE_CLOCKWISE: return "VK_FRONT_FACE_CLOCKWISE";
        default: return "Unknown(" + std::to_string(face) + ")";
        }
    }

    static std::string GetVkSampleCountFlagBitsString(VkSampleCountFlagBits samples) {
        switch (samples) {
        case VK_SAMPLE_COUNT_1_BIT: return "VK_SAMPLE_COUNT_1_BIT";
        case VK_SAMPLE_COUNT_2_BIT: return "VK_SAMPLE_COUNT_2_BIT";
        case VK_SAMPLE_COUNT_4_BIT: return "VK_SAMPLE_COUNT_4_BIT";
        case VK_SAMPLE_COUNT_8_BIT: return "VK_SAMPLE_COUNT_8_BIT";
        case VK_SAMPLE_COUNT_16_BIT: return "VK_SAMPLE_COUNT_16_BIT";
        case VK_SAMPLE_COUNT_32_BIT: return "VK_SAMPLE_COUNT_32_BIT";
        case VK_SAMPLE_COUNT_64_BIT: return "VK_SAMPLE_COUNT_64_BIT";
        default: return "Unknown(" + std::to_string(samples) + ")";
        }
    }

    static std::string GetVkCompareOpString(VkCompareOp op) {
        switch (op) {
        case VK_COMPARE_OP_NEVER: return "VK_COMPARE_OP_NEVER";
        case VK_COMPARE_OP_LESS: return "VK_COMPARE_OP_LESS";
        case VK_COMPARE_OP_EQUAL: return "VK_COMPARE_OP_EQUAL";
        case VK_COMPARE_OP_LESS_OR_EQUAL: return "VK_COMPARE_OP_LESS_OR_EQUAL";
        case VK_COMPARE_OP_GREATER: return "VK_COMPARE_OP_GREATER";
        case VK_COMPARE_OP_NOT_EQUAL: return "VK_COMPARE_OP_NOT_EQUAL";
        case VK_COMPARE_OP_GREATER_OR_EQUAL: return "VK_COMPARE_OP_GREATER_OR_EQUAL";
        case VK_COMPARE_OP_ALWAYS: return "VK_COMPARE_OP_ALWAYS";
        default: return "Unknown(" + std::to_string(op) + ")";
        }
    }

    static std::string GetVkStencilOpString(VkStencilOp op) {
        switch (op) {
        case VK_STENCIL_OP_KEEP: return "VK_STENCIL_OP_KEEP";
        case VK_STENCIL_OP_ZERO: return "VK_STENCIL_OP_ZERO";
        case VK_STENCIL_OP_REPLACE: return "VK_STENCIL_OP_REPLACE";
        case VK_STENCIL_OP_INCREMENT_AND_CLAMP: return "VK_STENCIL_OP_INCREMENT_AND_CLAMP";
        case VK_STENCIL_OP_DECREMENT_AND_CLAMP: return "VK_STENCIL_OP_DECREMENT_AND_CLAMP";
        case VK_STENCIL_OP_INVERT: return "VK_STENCIL_OP_INVERT";
        case VK_STENCIL_OP_INCREMENT_AND_WRAP: return "VK_STENCIL_OP_INCREMENT_AND_WRAP";
        case VK_STENCIL_OP_DECREMENT_AND_WRAP: return "VK_STENCIL_OP_DECREMENT_AND_WRAP";
        default: return "Unknown(" + std::to_string(op) + ")";
        }
    }

    static std::string GetVkBlendFactorString(VkBlendFactor factor) {
        switch (factor) {
        case VK_BLEND_FACTOR_ZERO: return "VK_BLEND_FACTOR_ZERO";
        case VK_BLEND_FACTOR_ONE: return "VK_BLEND_FACTOR_ONE";
        case VK_BLEND_FACTOR_SRC_COLOR: return "VK_BLEND_FACTOR_SRC_COLOR";
        case VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR: return "VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR";
        case VK_BLEND_FACTOR_DST_COLOR: return "VK_BLEND_FACTOR_DST_COLOR";
        case VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR: return "VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR";
        case VK_BLEND_FACTOR_SRC_ALPHA: return "VK_BLEND_FACTOR_SRC_ALPHA";
        case VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA: return "VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA";
        default: return "Unknown(" + std::to_string(factor) + ")";
        }
    }

    static std::string GetVkBlendOpString(VkBlendOp op) {
        switch (op) {
        case VK_BLEND_OP_ADD: return "VK_BLEND_OP_ADD";
        case VK_BLEND_OP_SUBTRACT: return "VK_BLEND_OP_SUBTRACT";
        case VK_BLEND_OP_REVERSE_SUBTRACT: return "VK_BLEND_OP_REVERSE_SUBTRACT";
        case VK_BLEND_OP_MIN: return "VK_BLEND_OP_MIN";
        case VK_BLEND_OP_MAX: return "VK_BLEND_OP_MAX";
        default: return "Unknown(" + std::to_string(op) + ")";
        }
    }

    static std::string GetVkColorComponentFlagsString(VkColorComponentFlags flags) {
        if (flags == 0) return "None";
        std::string result;
        if (flags & VK_COLOR_COMPONENT_R_BIT) result += "VK_COLOR_COMPONENT_R_BIT | ";
        if (flags & VK_COLOR_COMPONENT_G_BIT) result += "VK_COLOR_COMPONENT_G_BIT | ";
        if (flags & VK_COLOR_COMPONENT_B_BIT) result += "VK_COLOR_COMPONENT_B_BIT | ";
        if (flags & VK_COLOR_COMPONENT_A_BIT) result += "VK_COLOR_COMPONENT_A_BIT | ";
        if (!result.empty()) result = result.substr(0, result.size() - 3);
        return result.empty() ? "Unknown(" + std::to_string(flags) + ")" : result;
    }

    static std::string GetVkLogicOpString(VkLogicOp op) {
        switch (op) {
        case VK_LOGIC_OP_CLEAR: return "VK_LOGIC_OP_CLEAR";
        case VK_LOGIC_OP_AND: return "VK_LOGIC_OP_AND";
        case VK_LOGIC_OP_AND_REVERSE: return "VK_LOGIC_OP_AND_REVERSE";
        case VK_LOGIC_OP_COPY: return "VK_LOGIC_OP_COPY";
        case VK_LOGIC_OP_AND_INVERTED: return "VK_LOGIC_OP_AND_INVERTED";
        case VK_LOGIC_OP_NO_OP: return "VK_LOGIC_OP_NO_OP";
        case VK_LOGIC_OP_XOR: return "VK_LOGIC_OP_XOR";
        case VK_LOGIC_OP_OR: return "VK_LOGIC_OP_OR";
        default: return "Unknown(" + std::to_string(op) + ")";
        }
    }

    static std::string GetVkFormatString(VkFormat format) {
        switch (format) {
        case VK_FORMAT_R8G8B8A8_UNORM: return "VK_FORMAT_R8G8B8A8_UNORM";
        case VK_FORMAT_R32G32B32_SFLOAT: return "VK_FORMAT_R32G32B32_SFLOAT";
        case VK_FORMAT_R32G32_SFLOAT: return "VK_FORMAT_R32G32_SFLOAT";
        case VK_FORMAT_R32_SFLOAT: return "VK_FORMAT_R32_SFLOAT";
            // Add more formats as needed
        default: return "Unknown(" + std::to_string(format) + ")";
        }
    }

    static std::string GetVkVertexInputRateString(VkVertexInputRate rate) {
        switch (rate) {
        case VK_VERTEX_INPUT_RATE_VERTEX: return "VK_VERTEX_INPUT_RATE_VERTEX";
        case VK_VERTEX_INPUT_RATE_INSTANCE: return "VK_VERTEX_INPUT_RATE_INSTANCE";
        default: return "Unknown(" + std::to_string(rate) + ")";
        }
    }

    static void DebugGPUIncludes(const GPUIncludes& includes, const std::string& indent) {
        std::cout << indent << "VertexPropertiesCount: " << includes.VertexPropertiesCount << std::endl;
        DebugDescriptorBufferInfoList(includes.VertexProperties, includes.VertexPropertiesCount, "VertexProperties", indent + "  ");

        std::cout << indent << "IndexPropertiesCount: " << includes.IndexPropertiesCount << std::endl;
        DebugDescriptorBufferInfoList(includes.IndexProperties, includes.IndexPropertiesCount, "IndexProperties", indent + "  ");

        std::cout << indent << "TransformPropertiesCount: " << includes.TransformPropertiesCount << std::endl;
        DebugDescriptorBufferInfoList(includes.TransformProperties, includes.TransformPropertiesCount, "TransformProperties", indent + "  ");

        std::cout << indent << "MeshPropertiesCount: " << includes.MeshPropertiesCount << std::endl;
        DebugDescriptorBufferInfoList(includes.MeshProperties, includes.MeshPropertiesCount, "MeshProperties", indent + "  ");

        std::cout << indent << "TexturePropertiesCount: " << includes.TexturePropertiesCount << std::endl;
        DebugDescriptorImageInfoList(includes.TextureProperties, includes.TexturePropertiesCount, indent + "  ");

        std::cout << indent << "MaterialPropertiesCount: " << includes.MaterialPropertiesCount << std::endl;
        DebugDescriptorBufferInfoList(includes.MaterialProperties, includes.MaterialPropertiesCount, "MaterialProperties", indent + "  ");
    }

    static void DebugShaderPipelineData(const ShaderPipelineData& data, const std::string& indent) {
        std::cout << indent << "ShaderCount: " << data.ShaderCount << std::endl;
        DebugShaderList(data.ShaderList, data.ShaderCount, indent + "  ");

        std::cout << indent << "DescriptorBindingCount: " << data.DescriptorBindingCount << std::endl;
        DebugDescriptorBindingsList(data.DescriptorBindingsList, data.DescriptorBindingCount, indent + "  ");

        std::cout << indent << "ShaderStructCount: " << data.ShaderStructCount << std::endl;
        DebugShaderStructList(data.ShaderStructList, data.ShaderStructCount, indent + "  ");

        std::cout << indent << "VertexInputBindingCount: " << data.VertexInputBindingCount << std::endl;
        DebugVertexInputBindingList(data.VertexInputBindingList, data.VertexInputBindingCount, indent + "  ");

        std::cout << indent << "VertexInputAttributeListCount: " << data.VertexInputAttributeListCount << std::endl;
        DebugVertexInputAttributeList(data.VertexInputAttributeList, data.VertexInputAttributeListCount, indent + "  ");

        std::cout << indent << "ShaderOutputCount: " << data.ShaderOutputCount << std::endl;
        DebugShaderVariableList(data.ShaderOutputList, data.ShaderOutputCount, "ShaderOutput", indent + "  ");

        std::cout << indent << "PushConstantCount: " << data.PushConstantCount << std::endl;
        DebugPushConstantList(data.PushConstantList, data.PushConstantCount, indent + "  ");
    }

    static void DebugShaderList(const char** shaderList, size_t count, const std::string& indent) {
        if (!shaderList || count == 0) {
            std::cout << indent << "ShaderList: (empty)" << std::endl;
            return;
        }

        for (size_t i = 0; i < count; ++i) {
            std::string shaderName = shaderList[i] ? shaderList[i] : "(null)";
            std::cout << indent << "[" << i << "] Shader: " << shaderName << std::endl;
        }
    }

    static void DebugDescriptorBindingsList(const ShaderDescriptorBinding* bindings, size_t count, const std::string& indent) {
        if (!bindings || count == 0) {
            std::cout << indent << "DescriptorBindingsList: (empty)" << std::endl;
            return;
        }

        for (size_t i = 0; i < count; ++i) {
            const ShaderDescriptorBinding& binding = bindings[i];
            std::cout << indent << "[" << i << "] DescriptorBinding:" << std::endl;
            std::cout << indent << "  Name: " << (binding.Name ? binding.Name : "(null)") << std::endl;
            std::cout << indent << "  Binding: " << binding.Binding << std::endl;
            std::cout << indent << "  DescriptorCount: " << binding.DescriptorCount << std::endl;
            std::cout << indent << "  ShaderStageFlags: " << GetVkShaderStageFlagsString(binding.ShaderStageFlags) << std::endl;
            std::cout << indent << "  DescriptorBindingType: " << GetDescriptorBindingPropertiesEnumString(binding.DescriptorBindingType) << std::endl;
            std::cout << indent << "  DescripterType: " << GetVkDescriptorTypeString(binding.DescripterType) << std::endl;
            std::cout << indent << "  DescriptorImageInfo: 0x" << std::hex << std::setw(16) << std::setfill('0')
                << (uint64_t)binding.DescriptorImageInfo << std::dec << std::endl;
            std::cout << indent << "  DescriptorBufferInfo: 0x" << std::hex << std::setw(16) << std::setfill('0')
                << (uint64_t)binding.DescriptorBufferInfo << std::dec << std::endl;
        }
    }

    static void DebugShaderStructList(const ShaderStruct* structs, size_t count, const std::string& indent) {
        if (!structs || count == 0) {
            std::cout << indent << "ShaderStructList: (empty)" << std::endl;
            return;
        }

        for (size_t i = 0; i < count; ++i) {
            const ShaderStruct& shaderStruct = structs[i];
            std::cout << indent << "[" << i << "] ShaderStruct:" << std::endl;
            std::cout << indent << "  Name: " << (shaderStruct.Name ? shaderStruct.Name : "(null)") << std::endl;
            std::cout << indent << "  ShaderBufferSize: " << shaderStruct.ShaderBufferSize << std::endl;
            std::cout << indent << "  ShaderBufferVariableListCount: " << shaderStruct.ShaderBufferVariableListCount << std::endl;
            std::cout << indent << "  ShaderStructBufferId: " << shaderStruct.ShaderStructBufferId << std::endl;
            std::cout << indent << "  ShaderStructBuffer: 0x" << std::hex << std::setw(16) << std::setfill('0')
                << (uint64_t)shaderStruct.ShaderStructBuffer << std::dec << std::endl;
            DebugShaderVariableList(shaderStruct.ShaderBufferVariableList, shaderStruct.ShaderBufferVariableListCount,
                "ShaderBufferVariable", indent + "    ");
        }
    }

    static void DebugShaderVariableList(const ShaderVariable* variables, size_t count, const std::string& listName,
        const std::string& indent) {
        if (!variables || count == 0) {
            std::cout << indent << listName << "List: (empty)" << std::endl;
            return;
        }

        for (size_t i = 0; i < count; ++i) {
            const ShaderVariable& variable = variables[i];
            std::cout << indent << "[" << i << "] " << listName << ":" << std::endl;
            std::cout << indent << "  Name: " << (variable.Name ? variable.Name : "(null)") << std::endl;
            std::cout << indent << "  Size: " << variable.Size << std::endl;
            std::cout << indent << "  ByteAlignment: " << variable.ByteAlignment << std::endl;
            std::cout << indent << "  Value: 0x" << std::hex << std::setw(16) << std::setfill('0')
                << (uint64_t)variable.Value << std::dec << std::endl;
            std::cout << indent << "  MemberTypeEnum: " << GetShaderMemberTypeString(variable.MemberTypeEnum) << std::endl;
        }
    }

    static void DebugPushConstantList(const ShaderPushConstant* pushConstants, size_t count, const std::string& indent) {
        if (!pushConstants || count == 0) {
            std::cout << indent << "PushConstantList: (empty)" << std::endl;
            return;
        }

        for (size_t i = 0; i < count; ++i) {
            const ShaderPushConstant& pushConstant = pushConstants[i];
            std::cout << indent << "[" << i << "] PushConstant:" << std::endl;
            std::cout << indent << "  Name: " << (pushConstant.PushConstantName ? pushConstant.PushConstantName : "(null)") << std::endl;
            std::cout << indent << "  PushConstantSize: " << pushConstant.PushConstantSize << std::endl;
            std::cout << indent << "  PushConstantVariableListCount: " << pushConstant.PushConstantVariableListCount << std::endl;
            std::cout << indent << "  ShaderStageFlags: " << GetVkShaderStageFlagsString(pushConstant.ShaderStageFlags) << std::endl;
            std::cout << indent << "  PushConstantBuffer: 0x" << std::hex << std::setw(16) << std::setfill('0')
                << (uint64_t)pushConstant.PushConstantBuffer << std::dec << std::endl;
            std::cout << indent << "  GlobalPushContant: " << (pushConstant.GlobalPushContant ? "true" : "false") << std::endl;
            DebugShaderVariableList(pushConstant.PushConstantVariableList, pushConstant.PushConstantVariableListCount,
                "PushConstantVariable", indent + "    ");
        }
    }

    static void DebugVertexInputBindingList(const VkVertexInputBindingDescription* bindings, size_t count,
        const std::string& indent) {
        if (!bindings || count == 0) {
            std::cout << indent << "VertexInputBindingList: (empty)" << std::endl;
            return;
        }

        for (size_t i = 0; i < count; ++i) {
            const VkVertexInputBindingDescription& binding = bindings[i];
            std::cout << indent << "[" << i << "] VertexInputBinding:" << std::endl;
            std::cout << indent << "  Binding: " << binding.binding << std::endl;
            std::cout << indent << "  Stride: " << binding.stride << std::endl;
            std::cout << indent << "  InputRate: " << GetVkVertexInputRateString(binding.inputRate) << std::endl;
        }
    }

    static void DebugVertexInputAttributeList(const VkVertexInputAttributeDescription* attributes, size_t count,
        const std::string& indent) {
        if (!attributes || count == 0) {
            std::cout << indent << "VertexInputAttributeList: (empty)" << std::endl;
            return;
        }

        for (size_t i = 0; i < count; ++i) {
            const VkVertexInputAttributeDescription& attribute = attributes[i];
            std::cout << indent << "[" << i << "] VertexInputAttribute:" << std::endl;
            std::cout << indent << "  Location: " << attribute.location << std::endl;
            std::cout << indent << "  Binding: " << attribute.binding << std::endl;
            std::cout << indent << "  Format: " << GetVkFormatString(attribute.format) << std::endl;
            std::cout << indent << "  Offset: " << attribute.offset << std::endl;
        }
    }

    static void DebugDescriptorBufferInfoList(const VkDescriptorBufferInfo* infos, size_t count, const std::string& name,
        const std::string& indent) {
        if (!infos || count == 0) {
            std::cout << indent << name << "List: (empty)" << std::endl;
            return;
        }

        for (size_t i = 0; i < count; ++i) {
            const VkDescriptorBufferInfo& info = infos[i];
            std::cout << indent << "[" << i << "] " << name << ":" << std::endl;
            std::cout << indent << "  Buffer: 0x" << std::hex << std::setw(16) << std::setfill('0')
                << (uint64_t)info.buffer << std::dec << std::endl;
            std::cout << indent << "  Offset: " << info.offset << std::endl;
            std::cout << indent << "  Range: " << info.range << std::endl;
        }
    }

    static void DebugDescriptorImageInfoList(const VkDescriptorImageInfo* infos, size_t count, const std::string& indent) {
        if (!infos || count == 0) {
            std::cout << indent << "TexturePropertiesList: (empty)" << std::endl;
            return;
        }

        for (size_t i = 0; i < count; ++i) {
            const VkDescriptorImageInfo& info = infos[i];
            std::cout << indent << "[" << i << "] TextureProperties:" << std::endl;
            std::cout << indent << "  Sampler: 0x" << std::hex << std::setw(16) << std::setfill('0')
                << (uint64_t)info.sampler << std::dec << std::endl;
            std::cout << indent << "  ImageView: 0x" << std::hex << std::setw(16) << std::setfill('0')
                << (uint64_t)info.imageView << std::dec << std::endl;
            std::cout << indent << "  ImageLayout: " << info.imageLayout << std::endl;
        }
    }

    static void DebugViewportList(const VkViewport* viewports, size_t count, const std::string& indent) {
        if (!viewports || count == 0) {
            std::cout << indent << "ViewportList: (empty)" << std::endl;
            return;
        }

        for (size_t i = 0; i < count; ++i) {
            const VkViewport& viewport = viewports[i];
            std::cout << indent << "[" << i << "] Viewport:" << std::endl;
            std::cout << indent << "  X: " << viewport.x << std::endl;
            std::cout << indent << "  Y: " << viewport.y << std::endl;
            std::cout << indent << "  Width: " << viewport.width << std::endl;
            std::cout << indent << "  Height: " << viewport.height << std::endl;
            std::cout << indent << "  MinDepth: " << viewport.minDepth << std::endl;
            std::cout << indent << "  MaxDepth: " << viewport.maxDepth << std::endl;
        }
    }

    static void DebugScissorList(const VkRect2D* scissors, size_t count, const std::string& indent) {
        if (!scissors || count == 0) {
            std::cout << indent << "ScissorList: (empty)" << std::endl;
            return;
        }

        for (size_t i = 0; i < count; ++i) {
            const VkRect2D& scissor = scissors[i];
            std::cout << indent << "[" << i << "] Scissor:" << std::endl;
            std::cout << indent << "  Offset: (" << scissor.offset.x << ", " << scissor.offset.y << ")" << std::endl;
            std::cout << indent << "  Extent: (" << scissor.extent.width << ", " << scissor.extent.height << ")" << std::endl;
        }
    }

    static void DebugPipelineColorBlendAttachmentStateList(const VkPipelineColorBlendAttachmentState* states, size_t count,
        const std::string& indent) {
        if (!states || count == 0) {
            std::cout << indent << "PipelineColorBlendAttachmentStateList: (empty)" << std::endl;
            return;
        }

        for (size_t i = 0; i < count; ++i) {
            const VkPipelineColorBlendAttachmentState& state = states[i];
            std::cout << indent << "[" << i << "] PipelineColorBlendAttachmentState:" << std::endl;
            std::cout << indent << "  BlendEnable: " << (state.blendEnable ? "true" : "false") << std::endl;
            std::cout << indent << "  SrcColorBlendFactor: " << GetVkBlendFactorString(state.srcColorBlendFactor) << std::endl;
            std::cout << indent << "  DstColorBlendFactor: " << GetVkBlendFactorString(state.dstColorBlendFactor) << std::endl;
            std::cout << indent << "  ColorBlendOp: " << GetVkBlendOpString(state.colorBlendOp) << std::endl;
            std::cout << indent << "  SrcAlphaBlendFactor: " << GetVkBlendFactorString(state.srcAlphaBlendFactor) << std::endl;
            std::cout << indent << "  DstAlphaBlendFactor: " << GetVkBlendFactorString(state.dstAlphaBlendFactor) << std::endl;
            std::cout << indent << "  AlphaBlendOp: " << GetVkBlendOpString(state.alphaBlendOp) << std::endl;
            std::cout << indent << "  ColorWriteMask: " << GetVkColorComponentFlagsString(state.colorWriteMask) << std::endl;
        }
    }

    static void DebugPipelineInputAssemblyState(const VkPipelineInputAssemblyStateCreateInfo& state, const std::string& indent) {
        std::cout << indent << "Topology: " << GetVkPrimitiveTopologyString(state.topology) << std::endl;
        std::cout << indent << "PrimitiveRestartEnable: " << (state.primitiveRestartEnable ? "true" : "false") << std::endl;
    }

    static void DebugPipelineRasterizationState(const VkPipelineRasterizationStateCreateInfo& state, const std::string& indent) {
        std::cout << indent << "DepthClampEnable: " << (state.depthClampEnable ? "true" : "false") << std::endl;
        std::cout << indent << "RasterizerDiscardEnable: " << (state.rasterizerDiscardEnable ? "true" : "false") << std::endl;
        std::cout << indent << "PolygonMode: " << GetVkPolygonModeString(state.polygonMode) << std::endl;
        std::cout << indent << "CullMode: " << GetVkCullModeFlagsString(state.cullMode) << std::endl;
        std::cout << indent << "FrontFace: " << GetVkFrontFaceString(state.frontFace) << std::endl;
        std::cout << indent << "DepthBiasEnable: " << (state.depthBiasEnable ? "true" : "false") << std::endl;
        std::cout << indent << "DepthBiasConstantFactor: " << state.depthBiasConstantFactor << std::endl;
        std::cout << indent << "DepthBiasClamp: " << state.depthBiasClamp << std::endl;
        std::cout << indent << "DepthBiasSlopeFactor: " << state.depthBiasSlopeFactor << std::endl;
        std::cout << indent << "LineWidth: " << state.lineWidth << std::endl;
    }

    static void DebugPipelineMultisampleState(const VkPipelineMultisampleStateCreateInfo& state, const std::string& indent) {
        std::cout << indent << "RasterizationSamples: " << GetVkSampleCountFlagBitsString(state.rasterizationSamples) << std::endl;
        std::cout << indent << "SampleShadingEnable: " << (state.sampleShadingEnable ? "true" : "false") << std::endl;
        std::cout << indent << "MinSampleShading: " << state.minSampleShading << std::endl;
        std::cout << indent << "AlphaToCoverageEnable: " << (state.alphaToCoverageEnable ? "true" : "false") << std::endl;
        std::cout << indent << "AlphaToOneEnable: " << (state.alphaToOneEnable ? "true" : "false") << std::endl;
    }

    static void DebugPipelineDepthStencilState(const VkPipelineDepthStencilStateCreateInfo& state, const std::string& indent) {
        std::cout << indent << "DepthTestEnable: " << (state.depthTestEnable ? "true" : "false") << std::endl;
        std::cout << indent << "DepthWriteEnable: " << (state.depthWriteEnable ? "true" : "false") << std::endl;
        std::cout << indent << "DepthCompareOp: " << GetVkCompareOpString(state.depthCompareOp) << std::endl

            ;
        std::cout << indent << "DepthBoundsTestEnable: " << (state.depthBoundsTestEnable ? "true" : "false") << std::endl;
        std::cout << indent << "StencilTestEnable: " << (state.stencilTestEnable ? "true" : "false") << std::endl;
        std::cout << indent << "Front: [FailOp: " << GetVkStencilOpString(state.front.failOp) << ", PassOp: "
            << GetVkStencilOpString(state.front.passOp) << ", DepthFailOp: " << GetVkStencilOpString(state.front.depthFailOp)
            << ", CompareOp: " << GetVkCompareOpString(state.front.compareOp) << "]" << std::endl;
        std::cout << indent << "Back: [FailOp: " << GetVkStencilOpString(state.back.failOp) << ", PassOp: "
            << GetVkStencilOpString(state.back.passOp) << ", DepthFailOp: " << GetVkStencilOpString(state.back.depthFailOp)
            << ", CompareOp: " << GetVkCompareOpString(state.back.compareOp) << "]" << std::endl;
        std::cout << indent << "MinDepthBounds: " << state.minDepthBounds << std::endl;
        std::cout << indent << "MaxDepthBounds: " << state.maxDepthBounds << std::endl;
    }

    static void DebugPipelineColorBlendState(const VkPipelineColorBlendStateCreateInfo& state, const std::string& indent) {
        std::cout << indent << "LogicOpEnable: " << (state.logicOpEnable ? "true" : "false") << std::endl;
        std::cout << indent << "LogicOp: " << GetVkLogicOpString(state.logicOp) << std::endl;
        std::cout << indent << "BlendConstants: [" << state.blendConstants[0] << ", " << state.blendConstants[1]
            << ", " << state.blendConstants[2] << ", " << state.blendConstants[3] << "]" << std::endl;
    }
};