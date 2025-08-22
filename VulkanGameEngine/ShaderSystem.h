#pragma once
#include <ShaderCompiler.h>
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

	UnorderedMap<String, SpvReflectShaderModule> ShaderModuleMap;
	UnorderedMap<String, ShaderPushConstant> ShaderPushConstantSourceMap;
	UnorderedMap<String, ShaderPushConstant> GlobalPushContantShaderPushConstantMap;

public:

	ShaderSystem();
	~ShaderSystem();

	void StartUp();
	void VertexDataFromSpirv(const String& path);
	void AddShaderModule(const String& modulePath);
	void UpdateGlobalShaderBuffer(const String& pushConstantName);

	ShaderVariable* SearchGlobalShaderConstantVar(ShaderPushConstant& pushConstant, const String& varName);

	VkPipelineShaderStageCreateInfo CreateShader(VkDevice device, const String& path, VkShaderStageFlagBits shaderStages);

	ShaderPushConstant* GetGlobalShaderPushConstant(const String& pushConstantName);

	SpvReflectShaderModule& FindShaderModule(const String& shaderFile);
	ShaderPushConstant& FindShaderPushConstant(const String& shaderFile);

	bool ShaderModuleExists(const String& shaderFile);
	bool ShaderPushConstantExists(const String& pushConstantName);

	//void GetPushConstantData(const ShaderPushConstant& pushConstant);
};
extern ShaderSystem shaderSystem;
