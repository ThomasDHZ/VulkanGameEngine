#pragma once
#include <VulkanShader.h>
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
#include "FileSystem.h"

class ShaderSystem
{
private:
	Microsoft::WRL::ComPtr<IDxcUtils> dxc_utils;
	Microsoft::WRL::ComPtr<IDxcCompiler3> dxc_compiler;
	Microsoft::WRL::ComPtr<IDxcIncludeHandler> DefaultIncludeHandler;

	UnorderedMap<String, ShaderPipelineData> ShaderModuleMap;

public:
	UnorderedMap<String, ShaderPushConstant> ShaderPushConstantMap;
	UnorderedMap<String, ShaderStruct>  PipelineShaderStructPrototypeMap;
	UnorderedMap<int, ShaderStruct>  PipelineShaderStructMap;

	ShaderSystem();
	~ShaderSystem();

	void StartUp();
	void Destroy();

	ShaderPipelineData LoadShaderPipelineData(Vector<String> shaderPathList);
	void LoadShaderPipelineStructPrototypes(const Vector<String>& renderPassJsonList);

	ShaderPipelineData& FindShaderModule(const String& shaderFile);
	ShaderPushConstant& FindShaderPushConstant(const String& shaderFile);
	ShaderStruct FindShaderProtoTypeStruct(const String& structKey);
	ShaderStruct& FindShaderStruct(int vulkanBufferId);

	ShaderStruct CopyShaderStructProtoType(const String& structName);
	ShaderPushConstant* GetGlobalShaderPushConstant(const String& pushConstantName);

	ShaderVariable* SearchShaderStruct(ShaderStruct& shaderStruct, const String& varName);
	ShaderVariable* SearchGlobalShaderConstantVar(ShaderPushConstant* pushConstant, const char* varName);

	VkPipelineShaderStageCreateInfo CompileShader(const char* shaderFilePath, VkShaderStageFlagBits stage);

	void UpdateGlobalShaderBuffer(const String& pushConstantName);
	void UpdateShaderBuffer(uint vulkanBufferId);

	const bool ShaderModuleExists(const String& shaderFile) const;
	const bool ShaderPushConstantExists(const String& pushConstantName) const;
	const bool ShaderStructPrototypeExists(const String& structKey) const;
	const bool ShaderStructExists(uint vulkanBufferKey) const;
};
extern ShaderSystem shaderSystem;
