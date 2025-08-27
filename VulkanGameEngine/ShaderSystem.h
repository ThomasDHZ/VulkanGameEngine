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

	UnorderedMap<String, ShaderModule> ShaderModuleMap;
	UnorderedMap<String, ShaderPushConstant> ShaderPushConstantMap;

	UnorderedMap<uint,   ShaderStruct>  PipelineShaderStructMap;
	UnorderedMap<String, ShaderStruct>  PipelineShaderStructPrototypeMap;

	const Vector<VkDescriptorBufferInfo> GetVertexPropertiesBuffer();
	const Vector<VkDescriptorBufferInfo> GetIndexPropertiesBuffer();
	const Vector<VkDescriptorBufferInfo> GetGameObjectTransformBuffer();
	const Vector<VkDescriptorBufferInfo> GetMeshPropertiesBuffer(const VkGuid& levelLayerId);
	const Vector<VkDescriptorImageInfo>  GetTexturePropertiesBuffer(const VkGuid& renderPassId);

public:
	ShaderSystem();
	~ShaderSystem();


	void StartUp();
	void VertexDataFromSpirv(const String& path);
	ShaderModule AddShaderModule(const String& shaderPath, const VkGuid& renderPassId, const VkGuid& levelId);
	void UpdateGlobalShaderBuffer(const String& pushConstantName);
	void UpdateShaderBuffer(uint vulkanBufferId);
	void Destroy();

	ShaderVariable* SearchGlobalShaderConstantVar(ShaderPushConstant& pushConstant, const String& varName);

	VkPipelineShaderStageCreateInfo CreateShader(VkDevice device, const String& path, VkShaderStageFlagBits shaderStages);

	ShaderPushConstant* GetGlobalShaderPushConstant(const String& pushConstantName);

	ShaderModule& FindShaderModule(const String& shaderFile);
	ShaderPushConstant& FindShaderPushConstant(const String& shaderFile);
	ShaderStruct FindShaderProtoTypeStruct(const String& structKey);
	ShaderStruct& FindShaderStruct(uint vulkanBufferId);

	bool ShaderModuleExists(const String& shaderFile);
	bool ShaderPushConstantExists(const String& pushConstantName);
	bool ShaderStructPrototypeExists(const String& structKey);
	bool ShaderStructExists(uint vulkanBufferKey);
};
extern ShaderSystem shaderSystem;
