#pragma once
#include "DLL.h"
#include "JsonStruct.h"
#include <VulkanPipelineLoader.h>
#include "Collider2DComponent.h"
#include "Transform2DComponent.h"

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
}
