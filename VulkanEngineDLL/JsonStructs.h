#pragma once
#include <string>
#include <nlohmann/json.hpp>
#include "Typedef.h"
#include "JsonFunc.h"
#include "VkGuid.h"

enum RenderedTextureType
{
    ColorRenderedTexture,
    DepthRenderedTexture,
    InputAttachmentTexture,
    ResolveAttachmentTexture
};

enum DescriptorBindingPropertiesEnum
{
    kMeshPropertiesDescriptor,
    kTextureDescriptor,
    kMaterialDescriptor,
    kBRDFMapDescriptor,
    kIrradianceMapDescriptor,
    kPrefilterMapDescriptor,
    kCubeMapDescriptor,
    kEnvironmentDescriptor,
    kSunLightDescriptor,
    kDirectionalLightDescriptor,
    kPointLightDescriptor,
    kSpotLightDescriptor,
    kReflectionViewDescriptor,
    kDirectionalShadowDescriptor,
    kPointShadowDescriptor,
    kSpotShadowDescriptor,
    kViewTextureDescriptor,
    kViewDepthTextureDescriptor,
    kCubeMapSamplerDescriptor,
    kRotatingPaletteTextureDescriptor,
    kMathOpperation1Descriptor,
    kMathOpperation2Descriptor,
    kVertexDescsriptor,
    kIndexDescriptor,
    kTransformDescriptor
};

enum VertexTypeEnum
{
    NullVertex = 0,
    SpriteInstanceVertex = 1,
};

struct RenderedTextureInfoModel
{
    String RenderedTextureInfoName;
    VkImageCreateInfo ImageCreateInfo;
    VkSamplerCreateInfo SamplerCreateInfo;
    VkAttachmentDescription AttachmentDescription;
    RenderedTextureType TextureType;
};

struct RenderAreaModel
{
    VkRect2D RenderArea;
    bool UseDefaultRenderArea;
};

struct PipelineDescriptorModel
{
    uint BindingNumber;
    uint DstArrayElement;
    DescriptorBindingPropertiesEnum BindingPropertiesList;
    VkDescriptorType DescriptorType;
    VkShaderStageFlags StageFlags;
    VkSampler* pImmutableSamplers;
    VkBufferView* pTexelBufferView;
};

struct RenderPassBuildInfoModel
{
    VkGuid RenderPassId;
    bool IsRenderedToSwapchain;
    Vector<String> RenderPipelineList;
    Vector<RenderedTextureInfoModel> RenderedTextureInfoModelList;
    Vector<VkSubpassDependency> SubpassDependencyModelList;
    Vector<VkClearValue> ClearValueList;
    RenderAreaModel RenderArea;
};


