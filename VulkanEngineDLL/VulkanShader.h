#pragma once
extern "C"
{
#include "CShaderCompiler.h"
}
#include <Windows.h>
#include <dxcapi.h>
#include <wrl/client.h>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <vulkan/vulkan.h>
#include <fstream>
#include "TypeDef.h"
#include "JsonStructs.h"
#include "MemorySystem.h"
#include "JsonStruct.h"

DLL_EXPORT void Shader_StartUp();
DLL_EXPORT ShaderPiplineData Shader_GetShaderData(String* pipelineShaderPaths, size_t pipelineShaderCount);
DLL_EXPORT VkPipelineShaderStageCreateInfo Shader_CreateShader(VkDevice device, const String& path, VkShaderStageFlagBits shaderStages);
DLL_EXPORT void Shader_UpdateShaderBuffer(const GraphicsRenderer& renderer, VulkanBuffer& vulkanBuffer, ShaderStruct* shaderStruct, size_t shaderCount);
DLL_EXPORT ShaderStruct* Shader_LoadProtoTypeStructs(String* pipelineShaderPaths, size_t pipelineShaderCount, size_t& outProtoTypeStructCount);

DLL_EXPORT ShaderPushConstant Shader_SearchShaderConstBuffer(ShaderPushConstant* shaderPushConstantList, size_t shaderPushConstantCount, const String& constBufferName);
DLL_EXPORT ShaderDescriptorBinding Shader_SearchDescriptorBindings(ShaderDescriptorBinding* shaderDescriptorBindingList, size_t shaderDescriptorBindingsCount, const String& descriptorBindingName);
DLL_EXPORT ShaderStruct Shader_SearchShaderStructs(ShaderStruct* shaderStructList, size_t shaderStructCount, const String& structName);
DLL_EXPORT ShaderVariable* Shader_SearchShaderStructhVar(const ShaderStruct& shaderStruct, const String& varName);

DLL_EXPORT bool Shader_SearchShaderConstBufferExists(ShaderPushConstant* shaderPushConstantList, size_t shaderPushConstantCount, const String& constBufferName);
DLL_EXPORT bool Shader_SearchDescriptorBindingExists(ShaderDescriptorBinding* shaderDescriptorBindingList, size_t shaderDescriptorBindingsCount, const String& descriptorBindingName);
DLL_EXPORT bool Shader_SearchShaderStructExists(ShaderStruct* shaderStructList, size_t shaderStructCount, const String& structName);
DLL_EXPORT bool Shader_SearchShaderStructVarExists(const ShaderStruct& shaderStruct, const String& varName);

DLL_EXPORT const char* Renderer_GetShaderReflectError(SpvReflectResult result);
DLL_EXPORT void Shader_ShaderDestroy(ShaderPiplineData& shader);
DLL_EXPORT void Shader_DestroyShaderStructData(ShaderStruct* structList);
DLL_EXPORT void Shader_DestroyPushConstantBufferData(ShaderPushConstant* pushConstant);

void Shader_GetShaderInputVertexVariables(const SpvReflectShaderModule& module, Vector<VkVertexInputBindingDescription>& vertexInputBindingList, Vector<VkVertexInputAttributeDescription>& vertexInputAttributeList);
void Shader_GetShaderConstBuffer(const SpvReflectShaderModule& module, Vector<ShaderPushConstant>& shaderPushConstantList);
void Shader_GetShaderDescriptorBindings(const SpvReflectShaderModule& module, Vector<ShaderDescriptorBinding>& shaderDescriptorBindingList);
void Shader_GetShaderDescriptorSetInfo(const SpvReflectShaderModule& module, Vector<ShaderStruct>& shaderStruct);
Vector<SpvReflectInterfaceVariable*> Shader_GetShaderVertexInputVariables(const SpvReflectShaderModule& module);
Vector<SpvReflectInterfaceVariable*> Shader_GetShaderVertexOutputVariables(const SpvReflectShaderModule& module);
Vector<SpvReflectSpecializationConstant*> Shader_GetShaderSpecializationConstant(const SpvReflectShaderModule& module);
Vector<SpvReflectSpecializationConstant*> Shader_SearchShaderSpecializationConstant(Vector<SpvReflectSpecializationConstant*>& specializationConstantList, const char* searchString);
ShaderStruct Shader_GetShaderStruct(SpvReflectTypeDescription& shaderInfo);
void Shader_DestroyShaderBindingData(ShaderPiplineData& shader);

String Shader_ConvertLPCWSTRToString(LPCWSTR lpcwszStr);
void Shader_uint32ToUnsignedCharString(uint32 value, String& string);
VkShaderModule Shader_BuildGLSLShader(VkDevice device, const char* path, VkShaderStageFlagBits stage);

#define SPV_VULKAN_RESULT(call) { \
    SpvReflectResult result = (call); \
    if (result != SPV_REFLECT_RESULT_SUCCESS) { \
        fprintf(stderr, "Error in %s at %s:%d (%s): %s\n", \
                #call, __FILE__, __LINE__, __func__, Renderer_GetShaderReflectError(result)); \
    } \
}
