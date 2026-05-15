#pragma once
#include "Platform.h"
#include "TextureSystem.h"
#include "enum.h"

enum PushConstantResolverEnum
{
    kPushConst_None,
    kPushConst_MeshId,
    kPushConst_RenderPassResolution,
    kPushConst_PrefilterRoughness,
    kPushConst_SampleDelta
};

enum MeshTypeEnum
{
    kMesh_None,
    kMesh_SpriteMesh,
    kMesh_LevelMesh,
    kMesh_SkyBoxMesh,
    kMesh_LineMesh,
    kMesh_LevelEditorIconMesh,
    kMesh_FrameBuffer,
    kMesh_Undefined
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
    DescriptorBindingPropertiesEnum BindingPropertiesList;
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


struct RenderPassAttachmentTexture
{
    VkGuid                               RenderedTextureId = VkGuid();
    uint32                               MipMapCount = UINT32_MAX;
    RenderTextureTypeEnum                RenderTextureType = RenderType_UNKNOWN;
    Vector<RenderAttachmentTypeEnum>     RenderAttachmentTypes = Vector<RenderAttachmentTypeEnum>();
    VkFormat                             Format = VK_FORMAT_R8G8B8A8_UNORM;
    VkAttachmentLoadOp                   LoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    VkAttachmentStoreOp                  StoreOp = VK_ATTACHMENT_STORE_OP_STORE;
    VkSamplerCreateInfo                  SamplerCreateInfo = VkSamplerCreateInfo();
    VkImageLayout                        FinalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    bool                                 UseMipMaps = false;
};

struct PushConstantUpdateRule
{
    String                               Variable;
    PushConstantResolverEnum             SourceId;
    String                               Value;
    bool                                 ConstValue;
};

struct VulkanSubPass
{
    VkGuid                               RenderPassGuid;
    VkGuid                               PipelineGuid;
    MeshTypeEnum                         MeshType;
    std::optional<String>                ShaderPushConstant;
    Vector<PushConstantUpdateRule>       PushConstantUpdates;
    Vector<VkGuid>                       InputTextureList;
    Vector<VkGuid>                       OutputTextureList;
    bool                                 OffScreenFrameBuffer = false;
};

struct VulkanRenderPass
{
    RenderPassGuid                       RenderPassId = VkGuid();
    ivec2                                RenderPassResolution = ivec2(INT32_MAX, INT32_MAX);
    VkRenderPass                         RenderPass = VK_NULL_HANDLE;
    Vector<VkFramebuffer>                FrameBufferList;
    Vector<Vector<VulkanSubPass>>        VulkanSubPassList;
    Vector<VkClearValue>                 ClearValueList;
    VkSampleCountFlagBits                SampleCount = VK_SAMPLE_COUNT_1_BIT;
    bool                                 UseCubeMapMultiView = false;
    bool                                 IsCubeMapRenderPass = false;
};

struct VulkanSubPassLoader
{
    String                               Pipeline;
    MeshTypeEnum                         MeshType;
    std::optional<String>                ShaderPushConstant;
    Vector<PushConstantUpdateRule>       PushConstantUpdates;
    Vector<VkGuid>                       InputTextureList;
    Vector<VkGuid>                       OutputTextureList;
    bool                                 OffScreenRenderPass = false;
};

struct RenderPassLoader
{
    VkGuid                               RenderPassId = VkGuid();
    ivec2                                RenderPassResolution = ivec2(INT32_MAX, INT32_MAX);
    Vector<Vector<VulkanSubPassLoader>>  SubPassList;
    Vector<String>                       RenderPipelineList;
    Vector<RenderPassAttachmentTexture>  RenderAttachmentList;
    Vector<VkSubpassDependency>          SubpassDependencyList;
    Vector<VkClearValue>                 ClearValueList;
    VkSampleCountFlagBits                SampleCount = VK_SAMPLE_COUNT_1_BIT;
    bool                                 UseCubeMapMultiView = false;
    bool                                 IsCubeMapRenderPass = false;
};

struct RenderPassAttachementTextures
{
    size_t RenderPassTextureCount;
    Texture* RenderPassTexture;
    Texture* DepthTexture;
};

struct VulkanPipeline
{
    VkGuid RenderPipelineId;
    VkPipeline Pipeline = VK_NULL_HANDLE;
    VkPipelineCache PipelineCache = VK_NULL_HANDLE;
    VkPipelineLayout PipelineLayout = VK_NULL_HANDLE;
    Vector<VkDescriptorSetLayout> DescriptorSetLayoutList = Vector<VkDescriptorSetLayout>();
    Vector<VkDescriptorSet> DescriptorSetList = Vector<VkDescriptorSet>();
};

struct ShaderVariable
{
    String                          Name;
    size_t                          Size = 0;
    size_t                          ByteAlignment = 0;
    Vector<byte>                    Value;
    ShaderMemberType                MemberTypeEnum = shaderUnknown;
    bool                            ConstVariable = false;
};

struct ShaderStruct
{
    String                          Name;
    size_t			                ShaderBufferSize = 0;
    Vector<ShaderVariable>          ShaderBufferVariableList;
    int                             ShaderStructBufferId;
    Vector<byte>                    ShaderStructBuffer;
};

struct ShaderDescriptorSet
{
    String                          Name;
    uint32                          Binding;
    VkDescriptorType                DescripterType;
    Vector<ShaderStruct>            ShaderStructList;
};

struct ShaderDescriptorBinding
{
    String                          Name;
    uint32                          DescriptorSet = UINT32_MAX;
    uint32                          Binding = UINT32_MAX;
    size_t                          DescriptorCount;
    VkShaderStageFlags              ShaderStageFlags;
    DescriptorBindingPropertiesEnum DescriptorBindingType;
    VkDescriptorType                DescripterType;
    Vector<VkDescriptorImageInfo>   DescriptorImageInfo;
    Vector<VkDescriptorBufferInfo>  DescriptorBufferInfo;
};

struct ShaderPushConstant
{
    String                          PushConstantName;
    size_t			                PushConstantSize = 0;
    VkShaderStageFlags              ShaderStageFlags;
    Vector<ShaderVariable>          PushConstantVariableList;
    Vector<byte>                    PushConstantBuffer;
    bool			                GlobalPushConstant = false;
};

struct ShaderPipelineData
{
    Vector<String>                              ShaderList;
    Vector<ShaderDescriptorBinding>             DescriptorBindingsList;
    Vector<ShaderStruct>                        ShaderStructList;
    Vector<VkVertexInputBindingDescription>     VertexInputBindingList;
    Vector<VkVertexInputAttributeDescription>   VertexInputAttributeList;
    Vector<ShaderPushConstant>                  PushConstantList;
};

struct RenderPipelineLoader
{
    VkGuid PipelineId = VkGuid();
    VkGuid RenderPassId = VkGuid();
    VkGuid LevelId = VkGuid();
    uint32 SubPassId = UINT32_MAX;
    ivec2 RenderPassResolution = ivec2();
    VkRenderPass RenderPass = VK_NULL_HANDLE;
    ShaderPipelineData ShaderPiplineInfo;
    Vector<VkViewport> ViewportList;
    Vector<VkRect2D> ScissorList;
    Vector<VkPipelineColorBlendAttachmentState> PipelineColorBlendAttachmentStateList;
    VkPipelineInputAssemblyStateCreateInfo PipelineInputAssemblyStateCreateInfo = VkPipelineInputAssemblyStateCreateInfo();
    VkPipelineRasterizationStateCreateInfo PipelineRasterizationStateCreateInfo = VkPipelineRasterizationStateCreateInfo();
    VkPipelineMultisampleStateCreateInfo PipelineMultisampleStateCreateInfo = VkPipelineMultisampleStateCreateInfo();
    VkPipelineDepthStencilStateCreateInfo PipelineDepthStencilStateCreateInfo = VkPipelineDepthStencilStateCreateInfo();
    VkPipelineColorBlendStateCreateInfo PipelineColorBlendStateCreateInfoModel = VkPipelineColorBlendStateCreateInfo();
    bool UseDynamicColorWrite = false;
    bool UseCubeMapMultiview = false;
};