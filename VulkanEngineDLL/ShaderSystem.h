#pragma once
#include <Windows.h>
#include <dxcapi.h>
#include <wrl/client.h>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <vulkan/vulkan.h>
#include "TypeDef.h"
#include "MemorySystem.h"
#include "JsonStruct.h"
#include "BufferSystem.h"


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

#ifdef __cplusplus
extern "C" {
#endif
    //DLL_EXPORT VkShaderModule Shader_BuildGLSLShaderFile(VkDevice device, const char* path);
    //DLL_EXPORT bool Shader_BuildGLSLShaders(const char* command);
#ifdef __cplusplus
}
#endif

LPWSTR Shader_StringToLPWSTR(const String& str);

class ShaderSystem
{
private:
public:
    VkPipelineShaderStageCreateInfo LoadShader(VkDevice device, const char* filename, VkShaderStageFlagBits shaderStages);
    ShaderPipelineDataDLL LoadPipelineShaderData(const Vector<String>& pipelineShaderPaths);
    void LoadVertexInputVariables(const SpvReflectShaderModule& module, Vector<VkVertexInputBindingDescription>& vertexInputBindingList, Vector<VkVertexInputAttributeDescription>& vertexInputAttributeList);
    Vector<SpvReflectInterfaceVariable*> LoadVertexOutputVariables(const SpvReflectShaderModule& module);
    void LoadConstantBufferData(const SpvReflectShaderModule& module, Vector<ShaderPushConstantDLL>& shaderPushConstantList);
    void LoadDescriptorBindings(const SpvReflectShaderModule& module, Vector<ShaderDescriptorBindingDLL>& shaderDescriptorBindingList);
    void LoadShaderDescriptorSets(const SpvReflectShaderModule& module, Vector<ShaderStructDLL>& shaderStructList);
    void LoadShaderDescriptorSetInfo(const SpvReflectShaderModule& module, Vector<ShaderStructDLL>& shaderStructList);
    ShaderStructDLL LoadShaderPipelineStruct(const SpvReflectTypeDescription& shaderInfo);
    Vector<ShaderStructDLL> LoadProtoTypeStructs(const Vector<String>& pipelineShaderList);
    Vector<SpvReflectSpecializationConstant*> LoadSpecializationConstants(const SpvReflectShaderModule& module);

    void CompileGLSLShaders(const String& fileDirectory, const String& outputDirectory);
    void UpdatePushConstantBuffer(ShaderPushConstantDLL& pushConstantStruct);
    void UpdateShaderBuffer(uint vulkanBufferId);
    ShaderStructDLL CopyShaderStructProtoType(const String& structName);

    Vector<SpvReflectSpecializationConstant*> FindShaderSpecializationConstant(const Vector<SpvReflectSpecializationConstant*>& specializationConstantList, const String& searchString);
    ShaderPipelineDataDLL FindShaderModule(const String& shaderFile);
    ShaderStructDLL       FindShaderProtoTypeStruct(const String& shaderKey);
    ShaderStructDLL       FindShaderStruct(int vulkanBufferId);
    ShaderVariableDLL     FindShaderPipelineStructVariable(ShaderStructDLL& shaderStruct, const String& variableName);

    bool ShaderModuleExists(const String& shaderFile);
    bool ShaderPushConstantExists(const String& pushConstantName);
    bool ShaderStructPrototypeExists(const String& structKey);
    bool ShaderPipelineStructExists(uint vulkanBufferKey);
    bool SearchShaderConstantBufferExists(const Vector<ShaderPushConstantDLL>& shaderPushConstantList, const String& constBufferName);
    bool SearchShaderDescriptorBindingExists(const Vector<ShaderDescriptorBindingDLL>& shaderDescriptorBindingList, const String& descriptorBindingName);
    bool SearchShaderPipelineStructExists(const Vector<ShaderStructDLL>& shaderStructList, const String& structName);

    void ShaderDestroy(ShaderPipelineDataDLL& shader);
    void DestroyShaderStructData(Vector<ShaderStructDLL>& shaderStructList);
    void DestroyPushConstantBufferData(Vector<ShaderPushConstantDLL>& pushConstant);

    void SetVariableDefaults(ShaderVariableDLL& shaderVariable);

public:
    UnorderedMap<String, ShaderPipelineDataDLL> ShaderModuleMap;
	UnorderedMap<String, ShaderPushConstantDLL> ShaderPushConstantMap;
	UnorderedMap<String, ShaderStructDLL>  PipelineShaderStructPrototypeMap;
	UnorderedMap<int, ShaderStructDLL>  PipelineShaderStructMap;

    ShaderSystem();
    ~ShaderSystem();
    
    DLL_EXPORT void LoadShaderPipelineStructPrototypes(const Vector<String>& shaderPathList);
    DLL_EXPORT void UpdateGlobalShaderBuffer(const String& pushConstantName);
    DLL_EXPORT ShaderPushConstantDLL FindShaderPushConstant(const String& shaderFile);
    DLL_EXPORT ShaderVariableDLL     FindShaderPushConstantStructVariable(ShaderPushConstantDLL& shaderPushConstant, const String& variableName);
    DLL_EXPORT void Destroy();
};
DLL_EXPORT ShaderSystem shaderSystem;