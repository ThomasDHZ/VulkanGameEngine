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

struct ShaderVariableDLL
{
    String                          Name;
    size_t                          Size = 0;
    size_t                          ByteAlignment = 0;
    void* Value =                   nullptr;
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
    Vector<ShaderVariableDLL>       PushConstantVariableList;
    void* PushConstantBuffer =      nullptr;
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