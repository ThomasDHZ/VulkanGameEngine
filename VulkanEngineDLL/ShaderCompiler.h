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
#include <SPIRV-Reflect/spirv_reflect.h>
#include "MemorySystem.h"

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
	void*  Value = nullptr;
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
	void*			PushConstantBuffer = nullptr;
	bool			GlobalPushContant = false;
};

DLL_EXPORT void Shader_StartUp();
DLL_EXPORT SpvReflectShaderModule Shader_GetShaderData(const String& spvPath);
DLL_EXPORT VkPipelineShaderStageCreateInfo Shader_CreateShader(VkDevice device, const String& path, VkShaderStageFlagBits shaderStages);
DLL_EXPORT Vector<ShaderPushConstant> Shader_GetShaderConstBuffer(const SpvReflectShaderModule& module);
DLL_EXPORT const char* Renderer_GetShaderReflectError(SpvReflectResult result);


String Shader_ConvertLPCWSTRToString(LPCWSTR lpcwszStr);
void Shader_uint32ToUnsignedCharString(uint32 value, String& string);
VkShaderModule Shader_BuildGLSLShader(VkDevice device, const char* path, VkShaderStageFlagBits stage);

void Shader_GetShaderInputVertexVariables(const SpvReflectShaderModule& module, Vector<VkVertexInputBindingDescription>& vertexInputBindingList, Vector<VkVertexInputAttributeDescription>& vertexInputAttributeList);
Vector<ShaderVertexVariable> Shader_GetShaderOutputVertexVariables(const SpvReflectShaderModule& module);
Vector<ShaderDescriptorBinding> Shader_GetShaderDescriptorBindings(const SpvReflectShaderModule& module);
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
