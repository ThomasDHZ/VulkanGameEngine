#pragma once
#include <Platform.h>
#include <ShaderStructs.h>
#include <VulkanRenderPass.h>
#include <VulkanPipeline.h>
#include "TextureSystem.h"
#include "enum.h"
#include <ShaderEnums.h>

enum PushConstantResolverEnum
{
    kPushConst_None,
    kPushConst_MeshId,
    kPushConst_RenderPassResolution,
    kPushConst_PrefilterRoughness,
    kPushConst_SampleDelta
};



struct RenderedTextureInfoModel
{
    String RenderedTextureInfoName;
    VkImageCreateInfo ImageCreateInfo;
    VkSamplerCreateInfo SamplerCreateInfo;
    VkAttachmentDescription AttachmentDescription;
    RenderAttachmentTypeEnum TextureType;
};

struct PipelineDescriptorModel
{
    uint BindingNumber;
    uint DstArrayElement;
    DescriptorBindingTypeEnum BindingPropertiesList;
    VkDescriptorType DescriptorType;
    VkShaderStageFlags StageFlags;
    VkSampler* pImmutableSamplers;
    VkBufferView* pTexelBufferView;
};

struct RenderPassBuildInfoModel
{
    VkGuid RenderPassId;
    Vector<String> RenderPipelineList;
    Vector<RenderedTextureInfoModel> RenderedTextureInfoModelList;
    Vector<VkSubpassDependency> SubpassDependencyModelList;
    Vector<VkClearValue> ClearValueList;
};

struct BlendConstantsModel
{
    float Red;
    float Green;
    float Blue;
    float Alpha;
};

struct RenderPassAttachementTextures
{
    size_t RenderPassTextureCount;
    Texture* RenderPassTexture;
    Texture* DepthTexture;
};
