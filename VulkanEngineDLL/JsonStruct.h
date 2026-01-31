#pragma once
#include "Platform.h"
#include "TextureSystem.h"
#include "enum.h"

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
    bool IsRenderedToSwapchain;
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

struct VulkanRenderPass
{
    RenderPassGuid                       RenderPassId = VkGuid();
    uint32                               SubPassCount = UINT32_MAX;
    VkSampleCountFlagBits                SampleCount = VK_SAMPLE_COUNT_1_BIT;
    VkRenderPass                         RenderPass = VK_NULL_HANDLE;
    Vector<VkGuid>                       InputTextureIdList;
    Vector<VkFramebuffer>                FrameBufferList;
    Vector<VkClearValue>                 ClearValueList;
    Vector<VkSubpassDependency>          SubpassDependencyModelList;
    ivec2                                RenderPassResolution = ivec2();
    uint                                 MaxPushConstantSize = 0;
    bool                                 UseDefaultSwapChainResolution = true;
    bool                                 IsRenderedToSwapchain = false;
    bool                                 UseCubeMapMultiView = false;
};

struct RenderPassAttachmentTexture
{
    uint32                               MipMapCount = UINT32_MAX;
    RenderTextureTypeEnum                RenderTextureType = RenderType_UNKNOWN;
    Vector<RenderAttachmentTypeEnum>     RenderAttachmentTypes = Vector<RenderAttachmentTypeEnum>();
    VkFormat                             Format = VK_FORMAT_R8G8B8A8_UNORM;
    VkAttachmentLoadOp                   LoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    VkAttachmentStoreOp                  StoreOp = VK_ATTACHMENT_STORE_OP_STORE;
    VkSamplerCreateInfo                  SamplerCreateInfo = VkSamplerCreateInfo();
    VkImageLayout                        FinalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    bool                                 UseSampler = true;
    bool                                 UseMipMaps = false;
    bool                                 IsCubeMapAttachment = false;
    bool                                 IsTextureToExport = false;
};

struct RenderPassLoader
{
    VkGuid                               RenderPassId;
    uint32                               SubPassCount = UINT32_MAX;
    uint32                               RenderPassWidth = UINT32_MAX;
    uint32                               RenderPassHeight = UINT32_MAX;
    bool                                 UseDefaultSwapChainResolution = true;
    bool                                 UseCubeMapMultiView = false;
    bool                                 IsRenderedToSwapchain = false;
    VkSampleCountFlagBits                SampleCount = VK_SAMPLE_COUNT_1_BIT;
    Vector<String>                       RenderPipelineList;
    Vector<VkGuid>                       InputTextureList;
    Vector<RenderPassAttachmentTexture>  RenderAttachmentList;
    Vector<VkSubpassDependency>          SubpassDependencyModelList;
    Vector<VkClearValue>                 ClearValueList;
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
    VkDescriptorPool DescriptorPool = VK_NULL_HANDLE;
    Vector<VkDescriptorSetLayout> DescriptorSetLayoutList = Vector<VkDescriptorSetLayout>();
    Vector<VkDescriptorSet> DescriptorSetList = Vector<VkDescriptorSet>();
    VkPipeline Pipeline = VK_NULL_HANDLE;
    VkPipelineLayout PipelineLayout = VK_NULL_HANDLE;
    VkPipelineCache PipelineCache = VK_NULL_HANDLE;
};

struct ShaderVariableDLL
{
    String                          Name;
    size_t                          Size = 0;
    size_t                          ByteAlignment = 0;
    Vector<byte>                    Value;
    ShaderMemberType                MemberTypeEnum = shaderUnknown;
};

struct ShaderStructDLL
{
    String                          Name;
    size_t			                ShaderBufferSize = 0;
    Vector<ShaderVariableDLL>       ShaderBufferVariableList;
    int                             ShaderStructBufferId;
    Vector<byte>                    ShaderStructBuffer;
};

struct ShaderDescriptorSetDLL
{
    String                          Name;
    uint32                          Binding;
    VkDescriptorType                DescripterType;
    Vector<ShaderStructDLL>         ShaderStructList;
};

struct ShaderDescriptorBindingDLL
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

struct ShaderPushConstantDLL
{
    String                          PushConstantName;
    size_t			                PushConstantSize = 0;
    VkShaderStageFlags              ShaderStageFlags;
    Vector<ShaderVariableDLL>       PushConstantVariableList;
    Vector<byte>                    PushConstantBuffer;
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
    uint32 SubPassId = UINT32_MAX;
    ivec2 RenderPassResolution = ivec2();
    VkRenderPass RenderPass = VK_NULL_HANDLE;
    ShaderPipelineDataDLL ShaderPiplineInfo;
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