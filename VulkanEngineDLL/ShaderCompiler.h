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
#include <SPIRV-Reflect/spirv_reflect.h>

DLL_EXPORT void Shader_StartUp();
DLL_EXPORT void Shader_VertexDataFromSpirv(const Vector<byte>& spirvCode);
DLL_EXPORT VkPipelineShaderStageCreateInfo Shader_CreateShader(VkDevice device, const String& path, VkShaderStageFlagBits shaderStages);

String Shader_ConvertLPCWSTRToString(LPCWSTR lpcwszStr);
void Shader_uint32ToUnsignedCharString(uint32 value, String& string);
VkShaderModule Shader_BuildGLSLShader(VkDevice device, const char* path, VkShaderStageFlagBits stage);