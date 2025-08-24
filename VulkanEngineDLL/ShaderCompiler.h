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
DLL_EXPORT ShaderModule Shader_GetShaderData(const String& spvPath);
DLL_EXPORT VkPipelineShaderStageCreateInfo Shader_CreateShader(VkDevice device, const String& path, VkShaderStageFlagBits shaderStages);
DLL_EXPORT Vector<ShaderPushConstant> Shader_GetShaderConstBuffer(const SpvReflectShaderModule& module);
DLL_EXPORT const char* Renderer_GetShaderReflectError(SpvReflectResult result);


String Shader_ConvertLPCWSTRToString(LPCWSTR lpcwszStr);
void Shader_uint32ToUnsignedCharString(uint32 value, String& string);
VkShaderModule Shader_BuildGLSLShader(VkDevice device, const char* path, VkShaderStageFlagBits stage);

void Shader_GetShaderInputVertexVariables(const SpvReflectShaderModule& module, Vector<VkVertexInputBindingDescription>& vertexInputBindingList, Vector<VkVertexInputAttributeDescription>& vertexInputAttributeList);
Vector<ShaderVertexVariable> Shader_GetShaderOutputVertexVariables(const SpvReflectShaderModule& module);
Vector<ShaderDescriptorBinding> Shader_GetShaderDescriptorBindings(const SpvReflectShaderModule& module);
Vector<ShaderDescriptorSet> Shader_GetShaderDescriptorSet(const SpvReflectShaderModule& module);
Vector<SpvReflectInterfaceVariable*> Shader_GetShaderVertexInputVariables(const SpvReflectShaderModule& module);
Vector<SpvReflectInterfaceVariable*> Shader_GetShaderVertexOutputVariables(const SpvReflectShaderModule& module);
Vector<SpvReflectSpecializationConstant*> Shader_GetShaderSpecializationConstant(const SpvReflectShaderModule& module);
Vector<SpvReflectSpecializationConstant*> Shader_SearchShaderSpecializationConstant(Vector<SpvReflectSpecializationConstant*>& specializationConstantList, const char* searchString);

void printSpecializationConstants(SpvReflectShaderModule& module);
SpvOp getSpecializationConstantOp(const SpvReflectShaderModule& module, uint32_t spirv_id);


#define SPV_VULKAN_RESULT(call) { \
    SpvReflectResult result = (call); \
    if (result != SPV_REFLECT_RESULT_SUCCESS) { \
        fprintf(stderr, "Error in %s at %s:%d (%s): %s\n", \
                #call, __FILE__, __LINE__, __func__, Renderer_GetShaderReflectError(result)); \
    } \
}
