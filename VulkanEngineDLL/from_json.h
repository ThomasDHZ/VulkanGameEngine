#pragma once
#include "DLL.h"
#include "JsonStruct.h"
#include <VulkanPipelineLoader.h>
#include "Collider2DComponent.h"
#include "Transform2DComponent.h"
#include <variant>

namespace nlohmann
{
    ENGINE_DLL_EXPORT void from_json(const json& j, VkExtent2D& extent);
    ENGINE_DLL_EXPORT void from_json(const json& j, VkExtent3D& extent);
    ENGINE_DLL_EXPORT void from_json(const json& j, VkOffset2D& offset);
    ENGINE_DLL_EXPORT void from_json(const json& j, VkOffset3D& offset);
    ENGINE_DLL_EXPORT void from_json(const json& j, VkImageCreateInfo& info);
    ENGINE_DLL_EXPORT void from_json(const json& j, VkSamplerCreateInfo& info);
    ENGINE_DLL_EXPORT void from_json(const json& j, VkAttachmentDescription& desc);
    ENGINE_DLL_EXPORT void from_json(const json& j, VkSubpassDependency& dep);
    ENGINE_DLL_EXPORT void from_json(const json& j, VkClearValue& clearValue);
    ENGINE_DLL_EXPORT void from_json(const json& j, VkRect2D& rect);
    ENGINE_DLL_EXPORT void from_json(const json& j, VkGuid& guid);
    ENGINE_DLL_EXPORT void from_json(const json& j, VkViewport& viewPort);
    ENGINE_DLL_EXPORT void from_json(const json& j, RenderPassAttachmentLoader& model);
    ENGINE_DLL_EXPORT void from_json(const json& j, RenderedTextureInfoModel& model);
    ENGINE_DLL_EXPORT void from_json(const json& j, PipelineDescriptorModel& model);
    ENGINE_DLL_EXPORT void from_json(const json& j, RenderPassBuildInfoModel& model);
    ENGINE_DLL_EXPORT void from_json(const json& j, VkVertexInputBindingDescription& model);
    ENGINE_DLL_EXPORT void from_json(const json& j, VkVertexInputAttributeDescription& model);
    ENGINE_DLL_EXPORT void from_json(const json& j, VkPipelineColorBlendAttachmentState& model);
    ENGINE_DLL_EXPORT void from_json(const json& j, VkPipelineColorBlendStateCreateInfo& model);
    ENGINE_DLL_EXPORT void from_json(const json& j, VkPipelineRasterizationStateCreateInfo& model);
    ENGINE_DLL_EXPORT void from_json(const json& j, VkPipelineMultisampleStateCreateInfo& model);
    ENGINE_DLL_EXPORT void from_json(const json& j, VkPipelineDepthStencilStateCreateInfo& model);
    ENGINE_DLL_EXPORT void from_json(const json& j, VkPipelineInputAssemblyStateCreateInfo& model);
    ENGINE_DLL_EXPORT void from_json(const json& j, VkDescriptorSetLayoutBinding& model);
    ENGINE_DLL_EXPORT void from_json(const json& j, PushConstantUpdateRule& model);
    ENGINE_DLL_EXPORT void from_json(const json& j, VulkanSubPassLoader& model);
    ENGINE_DLL_EXPORT void from_json(const json& j, RenderPassLoader& model);
    ENGINE_DLL_EXPORT void from_json(const json& j, VulkanPipelinePackage& model);
    ENGINE_DLL_EXPORT void from_json(const json& j, VulkanPipelineLoader& model);
    ENGINE_DLL_EXPORT void from_json(const json& j, BlendConstantsModel& model);
    ENGINE_DLL_EXPORT void from_json(const json& j, TextureLoader& model);
    ENGINE_DLL_EXPORT void from_json(const json& j, Collider2DComponent& model);
    ENGINE_DLL_EXPORT void from_json(const json& j, Transform2DComponent& model);
    ENGINE_DLL_EXPORT void from_json(const json& j, ShaderLoader& model);
    
    //template <typename T>
    //T GetPushConstantValue(const nlohmann::json& value)
    //{
    //    if constexpr (std::is_same_v<T, bool>) return value.is_array() ? value.at(0).get<int>() != 0 : value.get<bool>();
    //    else if constexpr (std::is_arithmetic_v<T>) return value.is_array() ? value.at(0).get<T>() : value.get<T>();
    //    else return value.get<T>();
    //}

    //std::variant<int, uint32, float, bool, ivec2, ivec3, ivec4, vec2, vec3, vec4, mat2, mat3, mat4>
    //LoadPushConstant(const nlohmann::json& update)
    //{
    //    const auto& value = update.at("Value");
    //    switch (update.at("VariableType").get<int>())
    //    {
    //        case 0: return GetPushConstantValue<int>(value);
    //        case 1: return GetPushConstantValue<uint32_t>(value);
    //        case 2: return GetPushConstantValue<float>(value);
    //        case 3: return GetPushConstantValue<bool>(value);
    //        case 4: return GetPushConstantValue<ivec2>(value);
    //        case 5: return GetPushConstantValue<ivec3>(value);
    //        case 6: return GetPushConstantValue<ivec4>(value);
    //        case 7: return GetPushConstantValue<vec2>(value);
    //        case 8: return GetPushConstantValue<vec3>(value);
    //        case 9: return GetPushConstantValue<vec4>(value);
    //        case 10: return GetPushConstantValue<mat2>(value);
    //        case 11: return GetPushConstantValue<mat3>(value);
    //        case 12: return GetPushConstantValue<mat4>(value);
    //        default: throw std::runtime_error("unsupported push-constant type");
    //    }
    //}
}
