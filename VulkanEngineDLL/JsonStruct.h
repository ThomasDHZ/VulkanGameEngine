#pragma once
#include "Typedef.h"
#include "JsonStructs.h"
#include <SPIRV-Reflect/spirv_reflect.h>

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
enum ShaderMemberType
{
    shaderUnkown,
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

struct ShaderVertexVariable
{
    String Name;
    uint32 Location;
    VkFormat Format;
};

struct ShaderVariable
{
    String Name;
    size_t Size = 0;
    size_t ByteAlignment = 0;
    void* Value = nullptr;
    ShaderMemberType  MemberTypeEnum = shaderUnkown;
};

struct ShaderStruct
{
    String Name;
    String ShaderBufferMemberName;
    SpvOp ShaderStructOp;
    size_t ShaderBufferVariableListCount;
    ShaderVariable* ShaderBufferVariableList;
};

//struct ShaderBuffer
//{
//	String Name;
//	SpvOp  ShaderBufferOp;
//	size_t ShaderStructListCount;
//	ShaderVariable* ShaderStructList;
//};

struct ShaderDescriptorBinding
{
    String Name;
    uint32 Binding;
    VkDescriptorType DescripterType;
    size_t ShaderStructListCount;
    ShaderStruct* ShaderStructList;
};

struct ShaderPushConstant
{
    String			StructName;
    String			PushConstantName;
    size_t			PushConstantSize = 0;
    size_t			PushConstantVariableListCount = 0;
    ShaderVariable* PushConstantVariableList = nullptr;
    void* PushConstantBuffer = nullptr;
    bool			GlobalPushContant = false;
};

struct ShaderModule
{
    String							   ShaderPath;
    SpvReflectShaderStageFlagBits      ShaderStage;
    size_t                             DescriptorBindingCount = 0;
    size_t                             DescriptorSetCount = 0;
    size_t                             VertexInputBindingCount = 0;
    size_t                             VertexInputAttributeListCount = 0;
    size_t							   ShaderOutputCount = 0;
    size_t                             PushConstantCount = 0;
    ShaderDescriptorBinding* DescriptorBindingsList = nullptr;
    SpvReflectDescriptorSet* DescriptorSetList = nullptr;
    VkVertexInputBindingDescription* VertexInputBindingList = nullptr;
    VkVertexInputAttributeDescription* VertexInputAttributeList = nullptr;
    ShaderVariable* ShaderOutputList = nullptr;
    ShaderPushConstant* PushConstantList = nullptr;
};

struct GPUIncludes
{
    size_t VertexPropertiesCount;
    size_t IndexPropertiesCount;
    size_t TransformPropertiesCount;
    size_t MeshPropertiesCount;
    size_t TexturePropertiesListCount;
    size_t MaterialPropertiesCount;
    VkDescriptorBufferInfo* VertexProperties;
    VkDescriptorBufferInfo* IndexProperties;
    VkDescriptorBufferInfo* TransformProperties;
    VkDescriptorBufferInfo* MeshProperties;
    VkDescriptorImageInfo* TexturePropertiesList;
    VkDescriptorBufferInfo* MaterialProperties;
};

struct RenderPipelineLoader
{
    VkGuid PipelineId;
    VkGuid RenderPassId;
    VkRenderPass RenderPass;
    GPUIncludes gpuIncludes;
    ShaderPushConstant PushConstant;
    ShaderModule VertexShaderModule;
    ShaderModule FragmentShaderModule;
    size_t ViewportCount = 0;
    size_t ScissorCount = 0;
    size_t PipelineColorBlendAttachmentStateCount = 0;
    size_t PipelineDescriptorModelsCount = 0;
    VkPipelineColorBlendAttachmentState* PipelineColorBlendAttachmentStateList = nullptr;
    VkPipelineInputAssemblyStateCreateInfo PipelineInputAssemblyStateCreateInfo;
    VkViewport* ViewportList = nullptr;
    VkRect2D* ScissorList = nullptr;
    VkPipelineRasterizationStateCreateInfo PipelineRasterizationStateCreateInfo;
    VkPipelineMultisampleStateCreateInfo PipelineMultisampleStateCreateInfo;
    VkPipelineDepthStencilStateCreateInfo PipelineDepthStencilStateCreateInfo;
    VkPipelineColorBlendStateCreateInfo PipelineColorBlendStateCreateInfoModel;
    PipelineDescriptorModel* PipelineDescriptorModelsList = nullptr;
    ivec2 RenderPassResolution;
};