#pragma once
#include "Typedef.h"
#include "BufferSystem.h"
#include <SPIRV-Reflect/spirv_reflect.h>
#include "TextureSystem.h"
#include "enum.h"


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

struct BlendConstantsModel
{
    float Red;
    float Green;
    float Blue;
    float Alpha;
};

struct VulkanRenderPass
{
    RenderPassGuid RenderPassId = VkGuid();
    VkSampleCountFlagBits SampleCount = VK_SAMPLE_COUNT_1_BIT;
    VkRect2D RenderArea = VkRect2D();
    VkRenderPass RenderPass = VK_NULL_HANDLE;
    VkGuid* InputTextureIdList = nullptr;
    VkFramebuffer* FrameBufferList = nullptr;
    VkClearValue* ClearValueList = nullptr;
    size_t InputTextureIdListCount = 0;
    size_t FrameBufferCount = 0;
    size_t ClearValueCount = 0;
    VkCommandBuffer CommandBuffer = VK_NULL_HANDLE;
    ivec2 RenderPassResolution = ivec2();
    bool IsRenderedToSwapchain = false;
};

struct  RenderedTextureLoader
{
    VkGuid RenderedTextureId;
    VkImageCreateInfo ImageCreateInfo;
    VkSamplerCreateInfo SamplerCreateInfo;
    VkAttachmentDescription AttachmentDescription;
    RenderedTextureType TextureType;
    VkSampleCountFlagBits SampleCountOverride;
    bool UsingMipMaps;
};

struct RenderPassAttachementTextures
{
    size_t RenderPassTextureCount;
    Texture* RenderPassTexture;
    Texture* DepthTexture;
};

struct RenderPassLoader
{
    VkGuid RenderPassId;
    bool IsRenderedToSwapchain;
    Vector<String> RenderPipelineList;
    Vector<VkGuid> InputTextureList;
    Vector<RenderedTextureLoader> RenderedTextureInfoModelList;
    Vector<VkSubpassDependency> SubpassDependencyModelList;
    Vector<VkClearValue> ClearValueList;
    RenderAreaModel RenderArea;
};

struct GPUIncludes
{
    size_t VertexPropertiesCount = 0;
    size_t IndexPropertiesCount = 0;
    size_t TransformPropertiesCount = 0;
    size_t MeshPropertiesCount = 0;
    size_t TexturePropertiesCount = 0;
    size_t MaterialPropertiesCount = 0;
    VkDescriptorBufferInfo* VertexProperties = nullptr;
    VkDescriptorBufferInfo* IndexProperties = nullptr;
    VkDescriptorBufferInfo* TransformProperties = nullptr;
    VkDescriptorBufferInfo* MeshProperties = nullptr;
    VkDescriptorImageInfo* TextureProperties = nullptr;
    VkDescriptorBufferInfo* MaterialProperties = nullptr;
};

struct ShaderVariableDLL
{
    String                          Name;
    size_t                          Size = 0;
    size_t                          ByteAlignment = 0;
    void* Value = nullptr;
    ShaderMemberType                MemberTypeEnum = shaderUnknown;
};

struct ShaderStructDLL
{
    String                          Name;
    size_t			                ShaderBufferSize = 0;
    Vector<ShaderVariableDLL>          ShaderBufferVariableList;
    int                             ShaderStructBufferId;
    void* ShaderStructBuffer = nullptr;
};

struct ShaderDescriptorSetDLL
{
    String                          Name;
    uint32                          Binding;
    VkDescriptorType                DescripterType;
    Vector<ShaderStructDLL>            ShaderStructList;
};

struct ShaderDescriptorBindingDLL
{
    String                          Name;
    uint32                          Binding;
    size_t                          DescriptorCount;
    VkShaderStageFlags              ShaderStageFlags;
    DescriptorBindingPropertiesEnum DescriptorBindingType;
    VkDescriptorType                DescripterType;
    Vector<VkDescriptorImageInfo>   DescriptorImageInfo;
    Vector<VkDescriptorBufferInfo>  DescriptorBufferInfo;
};

struct ShaderPushConstantDLL
{
    String                          PushConstantName;
    size_t			                PushConstantSize = 0;
    VkShaderStageFlags              ShaderStageFlags;
    Vector<ShaderVariableDLL>          PushConstantVariableList;
    void* PushConstantBuffer = nullptr;
    bool			                GlobalPushContsant = false;
};

struct ShaderPipelineDataDLL
{
    Vector<String>                              ShaderList;
    Vector<ShaderDescriptorBindingDLL>             DescriptorBindingsList;
    Vector<ShaderStructDLL>                        ShaderStructList;
    Vector<VkVertexInputBindingDescription>     VertexInputBindingList;
    Vector<VkVertexInputAttributeDescription>   VertexInputAttributeList;
    Vector<ShaderPushConstantDLL>                  PushConstantList;
};

struct RenderPipelineLoader
{
    VkGuid PipelineId = VkGuid();
    VkGuid RenderPassId = VkGuid();
    VkGuid LevelId = VkGuid();
    VkRenderPass RenderPass = VK_NULL_HANDLE;
    ivec2 RenderPassResolution = ivec2();
    ShaderPipelineDataDLL ShaderPiplineInfo;
    size_t ViewportCount = 0;
    size_t ScissorCount = 0;
    size_t PipelineColorBlendAttachmentStateCount = 0;
    VkPipelineColorBlendAttachmentState* PipelineColorBlendAttachmentStateList = nullptr;
    VkPipelineInputAssemblyStateCreateInfo PipelineInputAssemblyStateCreateInfo = VkPipelineInputAssemblyStateCreateInfo();
    VkViewport* ViewportList = nullptr;
    VkRect2D* ScissorList = nullptr;
    VkPipelineRasterizationStateCreateInfo PipelineRasterizationStateCreateInfo = VkPipelineRasterizationStateCreateInfo();
    VkPipelineMultisampleStateCreateInfo PipelineMultisampleStateCreateInfo = VkPipelineMultisampleStateCreateInfo();
    VkPipelineDepthStencilStateCreateInfo PipelineDepthStencilStateCreateInfo = VkPipelineDepthStencilStateCreateInfo();
    VkPipelineColorBlendStateCreateInfo PipelineColorBlendStateCreateInfoModel = VkPipelineColorBlendStateCreateInfo();
};