#pragma once
#include "Platform.h"
#include "JsonStruct.h"
#include "MemorySystem.h"
#include "BufferSystem.h"
#include <cstdlib>

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
    const char* Name;
    size_t			ShaderBufferSize = 0;
    size_t          ShaderBufferVariableListCount = 0;
    ShaderVariable* ShaderBufferVariableList = nullptr;
    int             ShaderStructBufferId;
    void* ShaderStructBuffer = nullptr;
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
    const char* PushConstantName;
    size_t			   PushConstantSize = 0;
    size_t			   PushConstantVariableCount = 0;
    VkShaderStageFlags ShaderStageFlags;
    ShaderVariable* PushConstantVariableList;
    void* PushConstantBuffer = nullptr;
    bool			   GlobalPushContant = false;
};

struct ShaderPipelineData
{
    size_t PushConstantCount = 0;
    Vector<String> ShaderList;
    Vector<ShaderDescriptorBinding> DescriptorBindingsList;
    Vector<ShaderStruct> ShaderStructList;
    Vector<VkVertexInputBindingDescription> VertexInputBindingList;
    Vector<VkVertexInputAttributeDescription> VertexInputAttributeList;
    ShaderVariable* ShaderOutputList = nullptr;
    ShaderPushConstant* PushConstantList = nullptr;
};


class ShaderSystem
{
private:
    UnorderedMap<String, ShaderPipelineDataDLL> ShaderModuleMap;
	UnorderedMap<String, ShaderPushConstantDLL> ShaderPushConstantMap;
	UnorderedMap<String, ShaderStructDLL>       PipelineShaderStructPrototypeMap;

    void                                        LoadShaderVertexInputVariables(const SpvReflectShaderModule& module, Vector<VkVertexInputBindingDescription>& vertexInputBindingList, Vector<VkVertexInputAttributeDescription>& vertexInputAttributeList);
    Vector<SpvReflectInterfaceVariable*>        LoadShaderVertexOutputVariables(const SpvReflectShaderModule& module);
    void                                        LoadShaderConstantBufferData(const SpvReflectShaderModule& module, Vector<ShaderPushConstantDLL>& shaderPushConstantList);
    void                                        LoadShaderDescriptorBindings(const SpvReflectShaderModule& module, Vector<ShaderDescriptorBindingDLL>& shaderDescriptorBindingList);
    void                                        LoadShaderDescriptorSets(const SpvReflectShaderModule& module, Vector<ShaderStructDLL>& shaderStructList);
    void                                        LoadShaderDescriptorSetInfo(const SpvReflectShaderModule& module, Vector<ShaderStructDLL>& shaderStructList);
    ShaderStructDLL                             LoadShaderPipelineStruct(const SpvReflectTypeDescription& shaderInfo);
    Vector<ShaderVariableDLL>                   LoadShaderStructVariables(const SpvReflectTypeDescription& shaderInfo, size_t& returnBufferSize);
    Vector<ShaderStructDLL>                     LoadProtoTypeStructs(const Vector<String>& pipelineShaderList);
    Vector<SpvReflectSpecializationConstant*>   LoadShaderSpecializationConstants(const SpvReflectShaderModule& module);
    Vector<SpvReflectSpecializationConstant*>   FindShaderSpecializationConstant(const Vector<SpvReflectSpecializationConstant*>& specializationConstantList, const String& searchString);
    void                                        DestroyShaderStructData(Vector<ShaderStructDLL>& shaderStructList);
    void                                        DestroyPushConstantBufferData(Vector<ShaderPushConstantDLL>& pushConstant);
    void                                        SetVariableDefaults(ShaderVariableDLL& shaderVariable);

public:
	
    UnorderedMap<int, ShaderStructDLL>          PipelineShaderStructMap;

    ShaderSystem();
    ~ShaderSystem();
    
    DLL_EXPORT VkPipelineShaderStageCreateInfo  LoadShader(const char* filename, VkShaderStageFlagBits shaderStages);
    DLL_EXPORT ShaderPipelineDataDLL            LoadPipelineShaderData(const Vector<String>& pipelineShaderPaths);
    DLL_EXPORT void                             LoadShaderPipelineStructPrototypes(const Vector<String>& shaderPathList);
    DLL_EXPORT bool                             CompileShaders(const String& fileDirectory, const String& outputDirectory);
    DLL_EXPORT void                             UpdateGlobalShaderBuffer(const String& pushConstantName);
    DLL_EXPORT void                             UpdatePushConstantBuffer(ShaderPushConstantDLL& pushConstantStruct);
    DLL_EXPORT void                             UpdateShaderBuffer(uint vulkanBufferId);
    DLL_EXPORT ShaderStructDLL                  CopyShaderStructProtoType(const String& structName);
    DLL_EXPORT ShaderPipelineDataDLL            FindShaderModule(const String& shaderFile);
    DLL_EXPORT ShaderPushConstantDLL            FindShaderPushConstant(const String& shaderFile);
    DLL_EXPORT ShaderStructDLL                  FindShaderProtoTypeStruct(const String& shaderKey);
    DLL_EXPORT ShaderStructDLL                  FindShaderStruct(int vulkanBufferId);
    DLL_EXPORT ShaderVariableDLL                FindShaderPipelineStructVariable(ShaderStructDLL& shaderStruct, const String& variableName);
    DLL_EXPORT ShaderVariableDLL                FindShaderPushConstantStructVariable(ShaderPushConstantDLL& shaderPushConstant, const String& variableName);
    DLL_EXPORT bool                             ShaderModuleExists(const String& shaderFile);
    DLL_EXPORT bool                             ShaderPushConstantExists(const String& pushConstantName);
    DLL_EXPORT bool                             ShaderStructPrototypeExists(const String& structKey);
    DLL_EXPORT bool                             ShaderPipelineStructExists(uint vulkanBufferKey);
    DLL_EXPORT bool                             SearchShaderConstantBufferExists(const Vector<ShaderPushConstantDLL>& shaderPushConstantList, const String& constBufferName);
    DLL_EXPORT bool                             SearchShaderDescriptorBindingExists(const Vector<ShaderDescriptorBindingDLL>& shaderDescriptorBindingList, const String& descriptorBindingName);
    DLL_EXPORT bool                             SearchShaderPipelineStructExists(const Vector<ShaderStructDLL>& shaderStructList, const String& structName);
    DLL_EXPORT void                             Destroy();
};
extern DLL_EXPORT ShaderSystem shaderSystem;

#ifdef __cplusplus
extern "C" {
#endif
#ifdef __cplusplus
}
#endif