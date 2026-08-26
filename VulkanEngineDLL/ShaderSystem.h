#pragma once

#include <Platform.h>
#include <VulkanShader.h>
#include <VulkanPipeline.h>
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

    //UnorderedMap<VkGuid, Shader>                            VulkanShaderMap;
    //UnorderedMap<String, ShaderPipelineData>                ShaderModuleMap;
	UnorderedMap<String, ShaderStruct>                      PipelineShaderStructPrototypeMap;

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
	
	UnorderedMap<String, ShaderPushConstant>                ShaderPushConstantMap;
    UnorderedMap<int, ShaderStruct>                         PipelineShaderStructMap;
    
    VulkanShader                                  LoadShader(VkGuid& shaderGuid, const String& fileName);

     VkPipelineShaderStageCreateInfo              LoadShader(const char* filename, VkShaderStageFlagBits shaderStages);
     void                                         LoadShaderPipelineStructPrototypes(const Vector<String>& shaderPathList);
     Vector<SpvReflectSpecializationConstant*>    LoadShaderSpecializationConstants(const SpvReflectShaderModule& module);
     bool                                         CompileShaders(const String& fileDirectory, const String& outputDirectory);
     void                                         UpdatePushConstantBuffer(const String& pushConstantName);
     void                                         UpdatePushConstantBuffer(ShaderPushConstant& pushConstantStruct);
     ShaderStruct                                 CopyShaderStructProtoType(const String& structName);
     ShaderPushConstant&                          FindShaderPushConstant(const String& pushConstantName);
     ShaderStruct                                 FindShaderProtoTypeStruct(const String& shaderKey);
     ShaderStruct&                                FindShaderStruct(int vulkanBufferId);
     ShaderVariable&                              FindShaderPipelineStructVariable(ShaderStruct& shaderStruct, const String& variableName);
     ShaderVariable&                              FindShaderPushConstantStructVariable(ShaderPushConstant& shaderPushConstant, const String& variableName);
     Vector<SpvReflectSpecializationConstant*>    FindShaderSpecializationConstant(const Vector<SpvReflectSpecializationConstant*>& specializationConstantList, const String& searchString);
     bool                                         ShaderModuleExists(const String& shaderFile);
     bool                                         ShaderPushConstantExists(const String& pushConstantName);
     bool                                         ShaderStructPrototypeExists(const String& structKey);
     bool                                         ShaderPipelineStructExists(uint vulkanBufferKey);
     bool                                         SearchShaderConstantBufferExists(const Vector<ShaderPushConstant>& shaderPushConstantList, const String& constBufferName);
     bool                                         SearchShaderDescriptorBindingExists(const Vector<ShaderDescriptorBinding>& shaderDescriptorBindingList, const String& descriptorBindingName);
     bool                                         SearchShaderPipelineStructExists(const Vector<ShaderStruct>& shaderStructList, const String& structName);

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
extern  ShaderSystem& shaderSystem;
inline ShaderSystem& ShaderSystem::Get()
{
    static ShaderSystem instance;
    return instance;
}