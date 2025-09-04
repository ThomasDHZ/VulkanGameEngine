#pragma once
#include "Typedef.h"
#include "JsonStructs.h"
#include "VulkanBuffer.h"
#include <SPIRV-Reflect/spirv_reflect.h>

enum ShaderMemberType
{
    shaderUnknown,
    shaderInt,
    shaderUint,
    shaderFloat,
    shaderIvec2,
    shaderIvec3,
    shaderIvec4,
    shaderVec2,
    shaderVec3,
    shaderVec4,
    shaderMat2,
    shaderMat3,
    shaderMat4,
    shaderbool
};

struct BlendConstantsModel
{
    float Red;
    float Green;
    float Blue;
    float Alpha;
};

struct  RenderedTextureLoader
{
    VkGuid RenderedTextureId;
    String RenderedTextureInfoName;
    VkImageCreateInfo ImageCreateInfo;
    VkSamplerCreateInfo SamplerCreateInfo;
    VkAttachmentDescription AttachmentDescription;
    RenderedTextureType TextureType;
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
    size_t			PushConstantVariableListCount = 0;
    VkShaderStageFlags ShaderStageFlags;
    ShaderVariable* PushConstantVariableList = nullptr;
    void*           PushConstantBuffer = nullptr;
    bool			GlobalPushContant = false;
};

struct ShaderPipelineData
{
    size_t                             ShaderCount = 0;
    size_t                             DescriptorBindingCount = 0;
    size_t                             VertexInputBindingCount = 0;
    size_t                             VertexInputAttributeListCount = 0;
    size_t                             PushConstantCount = 0;
    ShaderDescriptorBinding*           DescriptorBindingsList = nullptr;
    VkVertexInputBindingDescription*   VertexInputBindingList = nullptr;
    VkVertexInputAttributeDescription* VertexInputAttributeList = nullptr;
    ShaderPushConstant*                PushConstantList = nullptr;
    const char**                       ShaderList = nullptr;
};

struct GPUIncludes
{
    size_t VertexPropertiesCount = 0;
    size_t IndexPropertiesCount = 0;
    size_t TransformPropertiesCount = 0;
    size_t MeshPropertiesCount = 0;
    size_t TexturePropertiesListCount = 0;
    size_t MaterialPropertiesCount = 0;
    VkDescriptorBufferInfo* VertexProperties = nullptr;
    VkDescriptorBufferInfo* IndexProperties = nullptr;
    VkDescriptorBufferInfo* TransformProperties = nullptr;
    VkDescriptorBufferInfo* MeshProperties = nullptr;
    VkDescriptorImageInfo* TexturePropertiesList = nullptr;
    VkDescriptorBufferInfo* MaterialProperties = nullptr;
};

struct RenderPipelineLoader
{
    VkGuid PipelineId;
    VkGuid RenderPassId;
    VkRenderPass RenderPass;
    GPUIncludes gpuIncludes;
    ShaderPushConstant PushConstant;
    ShaderPipelineData ShaderPiplineInfo;
    size_t ViewportCount = 0;
    size_t ScissorCount = 0;
    size_t PipelineColorBlendAttachmentStateCount = 0;
    VkPipelineColorBlendAttachmentState* PipelineColorBlendAttachmentStateList = nullptr;
    VkPipelineInputAssemblyStateCreateInfo PipelineInputAssemblyStateCreateInfo;
    VkViewport* ViewportList = nullptr;
    VkRect2D* ScissorList = nullptr;
    VkPipelineRasterizationStateCreateInfo PipelineRasterizationStateCreateInfo;
    VkPipelineMultisampleStateCreateInfo PipelineMultisampleStateCreateInfo;
    VkPipelineDepthStencilStateCreateInfo PipelineDepthStencilStateCreateInfo;
    VkPipelineColorBlendStateCreateInfo PipelineColorBlendStateCreateInfoModel;
    ivec2 RenderPassResolution;
};