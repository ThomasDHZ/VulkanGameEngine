#pragma once
#include "JsonStruct.h"

namespace nlohmann
{
    DLL_EXPORT void from_json(const json& j, VkExtent2D& extent);
    DLL_EXPORT void from_json(const json& j, VkExtent3D& extent);
    DLL_EXPORT void from_json(const json& j, VkOffset2D& offset);
    DLL_EXPORT void from_json(const json& j, VkOffset3D& offset);
    DLL_EXPORT void from_json(const json& j, VkImageCreateInfo& info);
    DLL_EXPORT void from_json(const json& j, VkSamplerCreateInfo& info);
    DLL_EXPORT void from_json(const json& j, VkAttachmentDescription& desc);
    DLL_EXPORT void from_json(const json& j, VkSubpassDependency& dep);
    DLL_EXPORT void from_json(const json& j, VkClearValue& clearValue);
    DLL_EXPORT void from_json(const json& j, VkRect2D& rect);
    DLL_EXPORT void from_json(const json& j, VkGuid& guid);
    DLL_EXPORT void from_json(const json& j, VkViewport& viewPort);
    DLL_EXPORT void from_json(const json& j, RenderPassAttachmentTexture& model);
    DLL_EXPORT void from_json(const json& j, RenderedTextureInfoModel& model);
    DLL_EXPORT void from_json(const json& j, PipelineDescriptorModel& model);
    DLL_EXPORT void from_json(const json& j, RenderPassBuildInfoModel& model);
    DLL_EXPORT void from_json(const json& j, VkVertexInputBindingDescription& model);
    DLL_EXPORT void from_json(const json& j, VkVertexInputAttributeDescription& model);
    DLL_EXPORT void from_json(const json& j, VkPipelineColorBlendAttachmentState& model);
    DLL_EXPORT void from_json(const json& j, VkPipelineColorBlendStateCreateInfo& model);
    DLL_EXPORT void from_json(const json& j, VkPipelineRasterizationStateCreateInfo& model);
    DLL_EXPORT void from_json(const json& j, VkPipelineMultisampleStateCreateInfo& model);
    DLL_EXPORT void from_json(const json& j, VkPipelineDepthStencilStateCreateInfo& model);
    DLL_EXPORT void from_json(const json& j, VkPipelineInputAssemblyStateCreateInfo& model);
    DLL_EXPORT void from_json(const json& j, VkDescriptorSetLayoutBinding& model);
    DLL_EXPORT void from_json(const json& j, RenderPassLoader& model);
    DLL_EXPORT void from_json(const json& j, RenderPipelineLoader& model);
    DLL_EXPORT void from_json(const json& j, BlendConstantsModel& model);
    DLL_EXPORT void from_json(const json& j, TextureLoader& model);
}
