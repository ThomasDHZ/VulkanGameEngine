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

	UnorderedMap<String, ShaderPiplineData> ShaderModuleMap;

public:
	UnorderedMap<String, ShaderPushConstant> ShaderPushConstantMap;
	UnorderedMap<String, ShaderStruct>  PipelineShaderStructPrototypeMap;
	UnorderedMap<int, ShaderStruct>  PipelineShaderStructMap;
	ShaderSystem();
	~ShaderSystem();

	
	void StartUp();
	void AddShaderModule(ShaderPiplineData& pipelineData, Vector<String> shaderPathList);
	void UpdateGlobalShaderBuffer(const String& pushConstantName);
	void UpdateShaderBuffer(uint vulkanBufferId);
	void Destroy();

	ShaderVariable* SearchShaderStruct(ShaderStruct& shaderStruct, const String& varName);
	ShaderVariable* SearchGlobalShaderConstantVar(ShaderPushConstant* pushConstant, const String& varName);
	VkPipelineShaderStageCreateInfo CreateShader(VkDevice device, const String& path, VkShaderStageFlagBits shaderStages);

	ShaderPushConstant* GetGlobalShaderPushConstant(const String& pushConstantName);
	void LoadShaderPipelineStructPrototypes(const Vector<String>& renderPassJsonList);

	ShaderPiplineData& FindShaderModule(const String& shaderFile);
	ShaderPushConstant& FindShaderPushConstant(const String& shaderFile);
	ShaderStruct FindShaderProtoTypeStruct(const String& structKey);
	ShaderStruct& FindShaderStruct(int vulkanBufferId);

	ShaderStruct CopyShaderStructProtoType(const String& structName);

	bool ShaderModuleExists(const String& shaderFile);
	bool ShaderPushConstantExists(const String& pushConstantName);
	bool ShaderStructPrototypeExists(const String& structKey);
	bool ShaderStructExists(uint vulkanBufferKey);
};
extern ShaderSystem shaderSystem;
