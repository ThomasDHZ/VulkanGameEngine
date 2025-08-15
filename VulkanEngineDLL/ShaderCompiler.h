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

struct ShaderVertexVariable
{
	String Name;
	uint32 Location;
	VkFormat Format;
};

struct ShaderVariable
{
	String Name;
	SpvOp  ShaderVarOp;
	uint   VectorCount;
	uint   ColumnCount;
	uint   RowCount;
	uint   MatrixStride;
	bool   IsSigned;
	bool   IsArray;
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
	String Name;
	size_t BufferSize;
	size_t ShaderBufferVariableListCount;
	ShaderVariable* ShaderBufferVariableList;
};

DLL_EXPORT void Shader_StartUp();
DLL_EXPORT void Shader_VertexDataFromSpirv(const Vector<byte>& spirvCode);
DLL_EXPORT VkPipelineShaderStageCreateInfo Shader_CreateShader(VkDevice device, const String& path, VkShaderStageFlagBits shaderStages);


String Shader_ConvertLPCWSTRToString(LPCWSTR lpcwszStr);
void Shader_uint32ToUnsignedCharString(uint32 value, String& string);
VkShaderModule Shader_BuildGLSLShader(VkDevice device, const char* path, VkShaderStageFlagBits stage);

Vector<ShaderVertexVariable> Shader_GetShaderInputVertexVariables(const SpvReflectShaderModule& module);
Vector<ShaderVertexVariable> Shader_GetShaderOutputVertexVariables(const SpvReflectShaderModule& module);
Vector<ShaderDescriptorBinding> Shader_GetShaderDescriptorBindings(const SpvReflectShaderModule& module);
Vector<ShaderPushConstant> Shader_GetShaderConstBuffer(const SpvReflectShaderModule& module);