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

public:
	ShaderSystem();
	~ShaderSystem();

	void StartUp();
	void VertexDataFromSpirv(const String& path);
	VkPipelineShaderStageCreateInfo CreateShader(VkDevice device, const String& path, VkShaderStageFlagBits shaderStages);
};
extern ShaderSystem shaderSystem;
