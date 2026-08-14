#pragma once
#include "JsonStruct.h"
#include <VulkanPipelineLoader.h>
#include "Collider2DComponent.h"
#include "Transform2DComponent.h"

namespace nlohmann
{
     void from_json(const json& j, VkExtent2D& extent);
     void from_json(const json& j, VkExtent3D& extent);
     void from_json(const json& j, VkOffset2D& offset);
     void from_json(const json& j, VkOffset3D& offset);
     void from_json(const json& j, VkImageCreateInfo& info);
     void from_json(const json& j, VkSamplerCreateInfo& info);
     void from_json(const json& j, VkAttachmentDescription& desc);
     void from_json(const json& j, VkSubpassDependency& dep);
     void from_json(const json& j, VkClearValue& clearValue);
     void from_json(const json& j, VkRect2D& rect);
     void from_json(const json& j, VkGuid& guid);
     void from_json(const json& j, VkViewport& viewPort);
     void from_json(const json& j, RenderPassAttachmentLoader& model);
     void from_json(const json& j, RenderedTextureInfoModel& model);
     void from_json(const json& j, PipelineDescriptorModel& model);
     void from_json(const json& j, RenderPassBuildInfoModel& model);
     void from_json(const json& j, VkVertexInputBindingDescription& model);
     void from_json(const json& j, VkVertexInputAttributeDescription& model);
     void from_json(const json& j, VkPipelineColorBlendAttachmentState& model);
     void from_json(const json& j, VkPipelineColorBlendStateCreateInfo& model);
     void from_json(const json& j, VkPipelineRasterizationStateCreateInfo& model);
     void from_json(const json& j, VkPipelineMultisampleStateCreateInfo& model);
     void from_json(const json& j, VkPipelineDepthStencilStateCreateInfo& model);
     void from_json(const json& j, VkPipelineInputAssemblyStateCreateInfo& model);
     void from_json(const json& j, VkDescriptorSetLayoutBinding& model);
     void from_json(const json& j, PushConstantUpdateRule& model);
     void from_json(const json& j, VulkanSubPassLoader& model);
     void from_json(const json& j, RenderPassLoader& model);
     void from_json(const json& j, VulkanPipelinePackageLoader& model);
     void from_json(const json& j, VulkanPipelineLoader& model);
     void from_json(const json& j, BlendConstantsModel& model);
     void from_json(const json& j, TextureLoader& model);
     void from_json(const json& j, Collider2DComponent& model);
     void from_json(const json& j, Transform2DComponent& model);
     void from_json(const json& j, ShaderLoader& model);
}
