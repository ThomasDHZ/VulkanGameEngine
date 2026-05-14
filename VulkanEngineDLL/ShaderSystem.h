#pragma once
#include "Platform.h"
#include "JsonStruct.h"
#include "MemorySystem.h"
#include "BufferSystem.h"
#include <cstdlib>

class ShaderSystem
{
public:
    static ShaderSystem& Get();

private:
    ShaderSystem() = default;
    ~ShaderSystem() = default;
    ShaderSystem(const ShaderSystem&) = delete;
    ShaderSystem& operator=(const ShaderSystem&) = delete;
    ShaderSystem(ShaderSystem&&) = delete;
    ShaderSystem& operator=(ShaderSystem&&) = delete;

    UnorderedMap<String, ShaderPipelineData> ShaderModuleMap;
	UnorderedMap<String, ShaderPushConstant> ShaderPushConstantMap;
	UnorderedMap<String, ShaderStruct>       PipelineShaderStructPrototypeMap;

    void                                                    LoadShaderVertexInputVariables(const SpvReflectShaderModule& module, Vector<VkVertexInputBindingDescription>& vertexInputBindingList, Vector<VkVertexInputAttributeDescription>& vertexInputAttributeList);
    Vector<SpvReflectInterfaceVariable*>                    LoadShaderVertexOutputVariables(const SpvReflectShaderModule& module);
    void                                                    LoadShaderConstantBufferData(const SpvReflectShaderModule& module, Vector<ShaderPushConstant>& shaderPushConstantList);
    void                                                    LoadShaderDescriptorBindings(const SpvReflectShaderModule& module, Vector<ShaderDescriptorBinding>& shaderDescriptorBindingList);
    void                                                    LoadShaderDescriptorSets(const SpvReflectShaderModule& module, Vector<ShaderStruct>& shaderStructList);
    void                                                    LoadShaderDescriptorSetInfo(const SpvReflectShaderModule& module, Vector<ShaderStruct>& shaderStructList);
    ShaderStruct                                            LoadShaderPipelineStruct(const SpvReflectTypeDescription& shaderInfo);
    Vector<ShaderVariable>                                  LoadShaderStructVariables(const SpvReflectTypeDescription& shaderInfo, size_t& returnBufferSize);
    Vector<ShaderStruct>                                    LoadProtoTypeStructs(const Vector<String>& pipelineShaderList);

public:
	
    UnorderedMap<int, ShaderStruct>             PipelineShaderStructMap;
    
    DLL_EXPORT VkPipelineShaderStageCreateInfo              LoadShader(const char* filename, VkShaderStageFlagBits shaderStages);
    DLL_EXPORT ShaderPipelineData                           LoadPipelineShaderData(const Vector<String>& pipelineShaderPaths);
    DLL_EXPORT void                                         LoadShaderPipelineStructPrototypes(const Vector<String>& shaderPathList);
    DLL_EXPORT Vector<SpvReflectSpecializationConstant*>    LoadShaderSpecializationConstants(const SpvReflectShaderModule& module);
    DLL_EXPORT bool                                         CompileShaders(const String& fileDirectory, const String& outputDirectory);
    DLL_EXPORT void                                         UpdatePushConstantBuffer(const String& pushConstantName);
    DLL_EXPORT void                                         UpdatePushConstantBuffer(ShaderPushConstant& pushConstantStruct);
    DLL_EXPORT void                                         UpdateShaderBuffer(ShaderStruct& shaderStruct, uint vulkanBufferId);
    DLL_EXPORT ShaderStruct                                 CopyShaderStructProtoType(const String& structName);
    DLL_EXPORT ShaderPipelineData                           FindShaderModule(const String& shaderFile);
    DLL_EXPORT ShaderPushConstant&                          FindShaderPushConstant(const String& pushConstantName);
    DLL_EXPORT ShaderStruct                                 FindShaderProtoTypeStruct(const String& shaderKey);
    DLL_EXPORT ShaderStruct&                                FindShaderStruct(int vulkanBufferId);
    DLL_EXPORT ShaderVariable&                              FindShaderPipelineStructVariable(ShaderStruct& shaderStruct, const String& variableName);
    DLL_EXPORT ShaderVariable&                              FindShaderPushConstantStructVariable(ShaderPushConstant& shaderPushConstant, const String& variableName);
    DLL_EXPORT Vector<SpvReflectSpecializationConstant*>    FindShaderSpecializationConstant(const Vector<SpvReflectSpecializationConstant*>& specializationConstantList, const String& searchString);
    DLL_EXPORT bool                                         ShaderModuleExists(const String& shaderFile);
    DLL_EXPORT bool                                         ShaderPushConstantExists(const String& pushConstantName);
    DLL_EXPORT bool                                         ShaderStructPrototypeExists(const String& structKey);
    DLL_EXPORT bool                                         ShaderPipelineStructExists(uint vulkanBufferKey);
    DLL_EXPORT bool                                         SearchShaderConstantBufferExists(const Vector<ShaderPushConstant>& shaderPushConstantList, const String& constBufferName);
    DLL_EXPORT bool                                         SearchShaderDescriptorBindingExists(const Vector<ShaderDescriptorBinding>& shaderDescriptorBindingList, const String& descriptorBindingName);
    DLL_EXPORT bool                                         SearchShaderPipelineStructExists(const Vector<ShaderStruct>& shaderStructList, const String& structName);

    template<typename T>
    void UpdatePushConstantValue(ShaderPushConstant& pushConst, const String& valueName, const T& value)
    {
        ShaderVariable& variable = FindShaderPushConstantStructVariable(pushConst, valueName);
        static_assert(std::is_trivially_copyable_v<T>, "Push constant type must be trivially copyable");
        if (variable.Value.size() != sizeof(T))
        {
            throw std::runtime_error(
                "Push constant size mismatch for '" + valueName + "': "
                "expected " + std::to_string(variable.Value.size()) + " bytes, "
                "got " + std::to_string(sizeof(T)) + " bytes (type: " + typeid(T).name() + ")"
            );
        }
        std::memcpy(variable.Value.data(), &value, variable.Value.size());
    }
};

extern DLL_EXPORT ShaderSystem& shaderSystem;
inline ShaderSystem& ShaderSystem::Get()
{
    static ShaderSystem instance;
    return instance;
}