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

struct RenderAreaModel
{
    VkRect2D RenderArea;
    bool UseSwapChainRenderArea;
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
    Vector<VkGuid> InputTextureIdList;
    Vector<VkFramebuffer> FrameBufferList;
    Vector<VkClearValue> ClearValueList;
    VkCommandBuffer CommandBuffer = VK_NULL_HANDLE;
    ivec2 RenderPassResolution = ivec2();
    bool IsRenderedToSwapchain = false;
};

struct  RenderAttachmentLoader
{
    VkGuid                       RenderedTextureId = VkGuid();
    uint32                       Width = -1;
    uint32                       Height = -1;
    uint32                       MipMapCount = -1;
    RenderTextureTypeEnum        RenderTextureType = RenderType_UNKNOWN;
    RenderAttachmentTypeEnum     RenderAttachmentType = ColorRenderedTexture;
    VkFormat                     Format = VK_FORMAT_R8G8B8A8_UNORM;
    VkSampleCountFlagBits        SampleCount = VK_SAMPLE_COUNT_1_BIT;
    VkAttachmentLoadOp           LoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    VkAttachmentStoreOp          StoreOp = VK_ATTACHMENT_STORE_OP_STORE;
    VkImageLayout                FinalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    VkSamplerCreateInfo          SamplerCreateInfo = VkSamplerCreateInfo();
    bool                         UseDefaultRenderArea = true;
    bool                         UseSampler = true;
    bool                         UseMipMaps = false;
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
    Vector<RenderAttachmentLoader> RenderAttachmentList;
    Vector<VkSubpassDependency> SubpassDependencyModelList;
    Vector<VkClearValue> ClearValueList;
    RenderAreaModel RenderArea;
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
};