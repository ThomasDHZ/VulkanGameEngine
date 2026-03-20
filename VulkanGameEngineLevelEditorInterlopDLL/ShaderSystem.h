#pragma once
#include <ShaderSystem.h>

struct ShaderVariableDLL
{
    const char*                     Name;
    size_t                          Size = 0;
    size_t                          ByteAlignment = 0;
    void*                           ValuePtr;
    size_t                          ValueSize;
    ShaderMemberType                MemberTypeEnum = shaderUnknown;
};

struct ShaderStructDLL
{
    const char* Name;
    size_t			                ShaderBufferSize = 0;
    ShaderVariableDLL               ShaderBufferVariableListPtr;
    size_t                          ShaderBufferVariableListSize;
    int                             ShaderStructBufferId;
    Vector<byte>                    ShaderStructBuffer;
};

struct ShaderDescriptorSetDLL
{
    const char* Name;
    uint32                          Binding;
    VkDescriptorType                DescripterType;
    Vector<ShaderStruct>            ShaderStructList;
};

struct ShaderDescriptorBindingDLL
{
    const char* Name;
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
    Vector<ShaderVariable>          PushConstantVariableList;
    Vector<byte>                    PushConstantBuffer;
    bool			                GlobalPushContsant = false;
};

struct ShaderPipelineDataDLL
{
    Vector<String>                              ShaderList;
    Vector<ShaderDescriptorBinding>             DescriptorBindingsList;
    Vector<ShaderStruct>                        ShaderStructList;
    Vector<VkVertexInputBindingDescription>     VertexInputBindingList;
    Vector<VkVertexInputAttributeDescription>   VertexInputAttributeList;
    Vector<ShaderPushConstant>                  PushConstantList;
};

#ifdef __cplusplus
extern "C" {
#endif
DLL_EXPORT VkPipelineShaderStageCreateInfo  ShaderSystem_LoadShader(const char* filename, VkShaderStageFlagBits shaderStages);
DLL_EXPORT ShaderPipelineData               ShaderSystem_LoadPipelineShaderData(const Vector<String>& pipelineShaderPaths);
DLL_EXPORT void                             ShaderSystem_LoadShaderPipelineStructPrototypes(const Vector<String>& shaderPathList);
DLL_EXPORT Vector<SpvReflectSpecializationConstant*>   ShaderSystem_LoadShaderSpecializationConstants(const SpvReflectShaderModule& module);
DLL_EXPORT bool                             ShaderSystem_CompileShaders(const String& fileDirectory, const String& outputDirectory);
DLL_EXPORT void                             ShaderSystem_UpdatePushConstantBuffer(const String& pushConstantName);
DLL_EXPORT void                             ShaderSystem_UpdatePushConstantBuffer(ShaderPushConstant& pushConstantStruct);
DLL_EXPORT void                             ShaderSystem_UpdateShaderBuffer(ShaderStruct& shaderStruct, uint vulkanBufferId);
DLL_EXPORT ShaderStruct                     ShaderSystem_CopyShaderStructProtoType(const String& structName);
DLL_EXPORT ShaderPipelineData               ShaderSystem_FindShaderModule(const String& shaderFile);
DLL_EXPORT ShaderPushConstant&				ShaderSystem_FindShaderPushConstant(const String& shaderFile);
DLL_EXPORT ShaderStruct                     ShaderSystem_FindShaderProtoTypeStruct(const String& shaderKey);
DLL_EXPORT ShaderStruct&					ShaderSystem_FindShaderStruct(int vulkanBufferId);
DLL_EXPORT ShaderVariable&					ShaderSystem_FindShaderPipelineStructVariable(ShaderStruct& shaderStruct, const String& variableName);
DLL_EXPORT ShaderVariable&					ShaderSystem_FindShaderPushConstantStructVariable(ShaderPushConstant& shaderPushConstant, const String& variableName);
DLL_EXPORT Vector<SpvReflectSpecializationConstant*>   ShaderSystem_FindShaderSpecializationConstant(const Vector<SpvReflectSpecializationConstant*>& specializationConstantList, const String& searchString);
DLL_EXPORT bool                             ShaderSystem_ShaderModuleExists(const String& shaderFile);
DLL_EXPORT bool                             ShaderSystem_ShaderPushConstantExists(const String& pushConstantName);
DLL_EXPORT bool                             ShaderSystem_ShaderStructPrototypeExists(const String& structKey);
DLL_EXPORT bool                             ShaderSystem_ShaderPipelineStructExists(uint vulkanBufferKey);
DLL_EXPORT bool                             ShaderSystem_SearchShaderConstantBufferExists(const Vector<ShaderPushConstant>& shaderPushConstantList, const String& constBufferName);
DLL_EXPORT bool                             ShaderSystem_SearchShaderDescriptorBindingExists(const Vector<ShaderDescriptorBinding>& shaderDescriptorBindingList, const String& descriptorBindingName);
DLL_EXPORT bool                             ShaderSystem_SearchShaderPipelineStructExists(const Vector<ShaderStruct>& shaderStructList, const String& structName);
#ifdef __cplusplus
}
#endif
