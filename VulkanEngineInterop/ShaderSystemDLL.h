#pragma once
#include "DLL.h"
#include <ShaderSystem.h>

struct ShaderVariableDLL
{
    const char*                     Name;
    size_t                          Size = 0;
    size_t                          ByteAlignment = 0;
    ShaderMemberTypeEnum            MemberTypeEnum = kShaderMember_Undefined;
    bool                            ConstVariable = false;
};

struct ShaderStructDLL
{
    const char*                     Name;
    ShaderVariable                  ShaderBufferVariableList;
    size_t                          ShaderBufferVariableCount;
};

struct ShaderDescriptorSetDLL
{
    const char*                     Name;
    uint32                          Binding;
    VkDescriptorType                DescripterType;
    ShaderStructDLL*                ShaderStructList;
    size_t                          ShaderStructCount;
};

struct ShaderPushConstantDLL
{
    const char*                     PushConstantName;
    size_t			                PushConstantSize = 0;
    VkShaderStageFlags              ShaderStageFlags;
    ShaderVariableDLL*              PushConstantVariableList;
    bool			                GlobalPushConstant = false;
    size_t                          PushConstantVariableCount;
};

struct ShaderDescriptorBindingDLL
{
    const char*                     Name;
    uint32                          DescriptorSet = UINT32_MAX;
    uint32                          Binding = UINT32_MAX;
    VkShaderStageFlags              ShaderStageFlags;
    SpvReflectDescriptorType        DescriptorBindingType;
    VkDescriptorType                DescripterType;
};

struct VulkanShaderDLL
{
    VkShaderModule ShaderModule;
    VkShaderStageFlagBits ShaderStages;
    ShaderPushConstantDLL PushConstant;
    VkVertexInputAttributeDescription* InputVertexAttributeList;
    VkVertexInputAttributeDescription* OutputVertexAttributeList;
    VkVertexInputBindingDescription* VertexInputBindingList;
    ShaderDescriptorBindingDLL* DescriptorBindingList;
    size_t InputVertexAttributeCount;
    size_t OutputVertexAttributeCount;
    size_t VertexInputBindingCount;
    size_t DescriptorBindingCount;
};

#ifdef __cplusplus
extern "C" {
#endif
	DLL_EXPORT VulkanShaderDLL ShaderSystem_LoadShader(const char* shaderFile);
#ifdef __cplusplus
}
#endif