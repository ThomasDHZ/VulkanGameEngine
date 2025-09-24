#pragma once
#include "Typedef.h"
#include "JsonStructs.h"
#include "VulkanBuffer.h"
#include <SPIRV-Reflect/spirv_reflect.h>
#include "Texture.h"
#include "enum.h"

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

struct TextureLoader
{
    String TextureFilePath;
    VkGuid TextureId;
    VkImageAspectFlags ImageType;
    TextureTypeEnum TextureType;
    bool UseMipMaps;
    VkImageCreateInfo ImageCreateInfo;
    VkSamplerCreateInfo SamplerCreateInfo;
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

struct ShaderVariable
{
    const char* Name;
    size_t Size = 0;
    size_t ByteAlignment = 0;
    void* Value = nullptr;
    ShaderMemberType  MemberTypeEnum = shaderUnknown;
};

struct ShaderStruct
{
    const char*     Name;
    size_t			ShaderBufferSize = 0;
    size_t          ShaderBufferVariableListCount = 0;
    ShaderVariable* ShaderBufferVariableList = nullptr;
    int             ShaderStructBufferId;
    void*           ShaderStructBuffer = nullptr;
};

struct ShaderDescriptorSet
{
    const char* Name;
    uint32 Binding;
    VkDescriptorType DescripterType;
    size_t ShaderStructListCount;
    ShaderStruct* ShaderStructList;
};

struct ShaderDescriptorBinding
{
    const char* Name;
    uint32 Binding;
    size_t DescriptorCount;
    VkShaderStageFlags ShaderStageFlags;
    DescriptorBindingPropertiesEnum DescriptorBindingType;
    VkDescriptorType DescripterType;
    VkDescriptorImageInfo* DescriptorImageInfo;
    VkDescriptorBufferInfo* DescriptorBufferInfo;
};

struct ShaderPushConstant
{
    const char*     PushConstantName;
    size_t			PushConstantSize = 0;
    size_t			PushConstantVariableCount = 0;
    VkShaderStageFlags ShaderStageFlags;
    ShaderVariable* PushConstantVariableList;
    void*           PushConstantBuffer = nullptr;
    bool			GlobalPushContant = false;
};

struct ShaderPipelineData
{
    size_t ShaderCount = 0;
    size_t DescriptorBindingCount = 0;
    size_t ShaderStructCount = 0;
    size_t VertexInputBindingCount = 0;
    size_t VertexInputAttributeListCount = 0;
    size_t ShaderOutputCount = 0;
    size_t PushConstantCount = 0;
    const char** ShaderList = nullptr;
    ShaderDescriptorBinding* DescriptorBindingsList = nullptr;
    ShaderStruct* ShaderStructList = nullptr;
    VkVertexInputBindingDescription* VertexInputBindingList = nullptr;
    VkVertexInputAttributeDescription* VertexInputAttributeList = nullptr;
    ShaderVariable* ShaderOutputList = nullptr;
    ShaderPushConstant* PushConstantList = nullptr;
};

struct ShaderVariableCPP
{
    String Name;
    size_t Size = 0;
    size_t ByteAlignment = 0;
    void* Value = nullptr;
    ShaderMemberType  MemberTypeEnum = shaderUnknown;
};

struct ShaderStructCPP
{
    String                 Name;
    size_t			       ShaderBufferSize = 0;
    int                    ShaderStructBufferId = 0;
    Vector<ShaderVariable> ShaderBufferVariableList;
    void*                  ShaderStructBuffer = nullptr;
};

struct ShaderDescriptorSetCPP
{
    String                  Name;
    uint32                  Binding;
    VkDescriptorType        DescripterType;
    Vector<ShaderStruct>    ShaderStructList;
};

struct ShaderDescriptorBindingCPP
{
    String                          Name;
    uint32                          Binding;
    size_t                          DescriptorCount;
    VkShaderStageFlags              ShaderStageFlags;
    DescriptorBindingPropertiesEnum DescriptorBindingType;
    VkDescriptorType                DescripterType;
    VkDescriptorImageInfo           DescriptorImageInfo;
    VkDescriptorBufferInfo          DescriptorBufferInfo;
};

struct ShaderPushConstantCPP
{
    String                  PushConstantName;
    size_t			        PushConstantSize = 0;
    VkShaderStageFlags      ShaderStageFlags;
    Vector<ShaderVariable>  PushConstantVariableList;
    void*                   PushConstantBuffer = nullptr;
    bool			        GlobalPushContant = false;
};

struct ShaderPipelineDataCPP
{
    size_t ShaderCount = 0;
    size_t DescriptorBindingCount = 0;
    size_t ShaderStructCount = 0;
    size_t VertexInputBindingCount = 0;
    size_t VertexInputAttributeListCount = 0;
    size_t ShaderOutputCount = 0;
    size_t PushConstantCount = 0;
    const char** ShaderList = nullptr;
    ShaderDescriptorBinding* DescriptorBindingsList = nullptr;
    ShaderStruct* ShaderStructList = nullptr;
    VkVertexInputBindingDescription* VertexInputBindingList = nullptr;
    VkVertexInputAttributeDescription* VertexInputAttributeList = nullptr;
    ShaderVariable* ShaderOutputList = nullptr;
    ShaderPushConstant* PushConstantList = nullptr;
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

struct RenderPipelineLoader
{
    VkGuid PipelineId = VkGuid();
    VkGuid RenderPassId = VkGuid();
    VkRenderPass RenderPass = VK_NULL_HANDLE;
    ivec2 RenderPassResolution = ivec2();
    GPUIncludes gpuIncludes = GPUIncludes();
    ShaderPipelineData ShaderPiplineInfo = ShaderPipelineData();
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