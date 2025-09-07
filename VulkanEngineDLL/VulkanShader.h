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
#include "TypeDef.h"
#include "JsonStructs.h"
#include "MemorySystem.h"
#include "JsonStruct.h"

#ifdef __cplusplus
extern "C" {
#endif
    DLL_EXPORT void Shader_StartUp();
    DLL_EXPORT ShaderPipelineData Shader_LoadPipelineShaderData(const char** pipelineShaderPaths, size_t pipelineShaderCount);
    DLL_EXPORT void Shader_UpdateShaderBuffer(const GraphicsRenderer& renderer, VulkanBuffer& vulkanBuffer, ShaderStruct* shaderStruct, size_t shaderCount);
    DLL_EXPORT void Shader_UpdatePushConstantBuffer(const GraphicsRenderer& renderer, ShaderPushConstant& pushConstantStruct);
    DLL_EXPORT ShaderStruct* Shader_LoadProtoTypeStructs(const char** pipelineShaderPaths, size_t pipelineShaderCount, size_t& outProtoTypeStructCount);
    DLL_EXPORT ShaderStruct Shader_CopyShaderStructPrototype(const ShaderStruct& shaderStructToCopy);
    DLL_EXPORT ShaderVariable* Shader_SearchShaderConstStructVar(ShaderPushConstant* pushConstant, const char* varName);
    DLL_EXPORT ShaderVariable* Shader_SearchShaderStructVar(ShaderStruct* shaderStruct, const char* varName);
    DLL_EXPORT void Shader_ShaderDestroy(ShaderPipelineData& shader);
    DLL_EXPORT void Shader_DestroyShaderStructData(ShaderStruct* shaderStruct);
    DLL_EXPORT void Shader_DestroyPushConstantBufferData(ShaderPushConstant* pushConstant);
    DLL_EXPORT void Shader_SetVariableDefaults(ShaderVariable& shaderVariable);
#ifdef __cplusplus
}
#endif

DLL_EXPORT VkPipelineShaderStageCreateInfo Shader_LoadShader(VkDevice device, const char* path, VkShaderStageFlagBits shaderStages);
DLL_EXPORT VkPipelineShaderStageCreateInfo Shader_CompileShader(VkDevice device, const char* shaderFilePath, VkShaderStageFlagBits shaderStages);
ShaderPushConstant* Shader_SearchShaderConstBuffer(ShaderPushConstant* shaderPushConstantList, size_t shaderPushConstantCount, const char* constBufferName);
//ShaderDescriptorBinding* Shader_SearchDescriptorBindings(ShaderDescriptorBinding* shaderDescriptorBindingList, size_t shaderDescriptorBindingsCount, const char* descriptorBindingName);
ShaderStruct* Shader_SearchShaderStructs(ShaderStruct* shaderStructList, size_t shaderStructCount, const char* structName);
Vector<SpvReflectInterfaceVariable*> Shader_GetShaderVertexInputVariables(const SpvReflectShaderModule& module);
Vector<SpvReflectInterfaceVariable*> Shader_GetShaderVertexOutputVariables(const SpvReflectShaderModule& module);
Vector<SpvReflectSpecializationConstant*> Shader_GetShaderSpecializationConstant(const SpvReflectShaderModule& module);
Vector<SpvReflectSpecializationConstant*> Shader_SearchShaderSpecializationConstant(Vector<SpvReflectSpecializationConstant*>& specializationConstantList, const char* searchString);
ShaderStruct Shader_GetShaderStruct(SpvReflectTypeDescription& shaderInfo);
void Shader_DestroyShaderBindingData(ShaderPipelineData& shader);
bool Shader_SearchShaderConstBufferExists(ShaderPushConstant* shaderPushConstantList, size_t shaderPushConstantCount, const char* constBufferName);
bool Shader_SearchDescriptorBindingExists(ShaderDescriptorBinding* shaderDescriptorBindingList, size_t shaderDescriptorBindingsCount, const char* descriptorBindingName);
bool Shader_SearchShaderStructExists(ShaderStruct* shaderStructList, size_t shaderStructCount, const char* structName);
bool Shader_SearchShaderStructVarExists(const ShaderStruct* shaderStruct, const char* varName);
void Shader_GetShaderInputVertexVariables(const SpvReflectShaderModule& module, Vector<VkVertexInputBindingDescription>& vertexInputBindingList, Vector<VkVertexInputAttributeDescription>& vertexInputAttributeList);
void Shader_GetShaderConstBuffer(const SpvReflectShaderModule& module, Vector<ShaderPushConstant>& shaderPushConstantList);
void Shader_GetShaderDescriptorBindings(const SpvReflectShaderModule& module, Vector<ShaderDescriptorBinding>& shaderDescriptorBindingList);
void Shader_GetShaderDescriptorSetInfo(const SpvReflectShaderModule& module, Vector<ShaderStruct>& shaderStruct);

VkShaderModule Shader_ReadGLSLShader(VkDevice device, const char* path, VkShaderStageFlagBits stage);
VkShaderModule Shader_CompileGLSLShader(VkDevice device, const char* shaderFilePath, VkShaderStageFlagBits stage);
Microsoft::WRL::ComPtr<IDxcBlob> Shader_CompileHLSLShader(VkDevice device, const String& path, Microsoft::WRL::ComPtr<IDxcCompiler3>& dxc_compiler, Microsoft::WRL::ComPtr<IDxcIncludeHandler>& defaultIncludeHandler, VkShaderStageFlagBits stage);

LPWSTR Shader_StringToLPWSTR(const String& str);
String Shader_ConvertLPCWSTRToString(LPCWSTR lpcwszStr);
void Shader_uint32ToUnsignedCharString(uint32 value, String& string);
const char* Renderer_GetShaderReflectError(SpvReflectResult result);

#define SPV_VULKAN_RESULT(call) { \
    SpvReflectResult result = (call); \
    if (result != SPV_REFLECT_RESULT_SUCCESS) { \
        fprintf(stderr, "Error in %s at %s:%d (%s): %s\n", \
                #call, __FILE__, __LINE__, __func__, Renderer_GetShaderReflectError(result)); \
    } \
}
