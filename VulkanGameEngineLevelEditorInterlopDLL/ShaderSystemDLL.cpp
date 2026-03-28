//#include "ShaderSystemDLL.h"
//#include <MemorySystem.h>
//
//ShaderVariable ShaderSystemDLL_ShaderVariableDLLToCPP(ShaderVariableDLL& shaderVariable)
//{
//	Vector<byte> shaderVariableBytList;
//	shaderVariableBytList.reserve(shaderVariable.ValueSize);
//	std::memcpy(shaderVariableBytList.data(), shaderVariable.ValuePtr, shaderVariable.ValueSize);
//
//	return ShaderVariable
//	{
//		.Name = shaderVariable.Name,
//		.Size = shaderVariable.Size,
//		.ByteAlignment = shaderVariable.ByteAlignment,
//		.Value = shaderVariableBytList,
//		.MemberTypeEnum = shaderVariable.MemberTypeEnum
//	};
//}
//
//ShaderVariableDLL ShaderSystemDLL_ShaderVariableCPPToDLL(ShaderVariable& shaderVariable)
//{
//	byte* variablePtr = memorySystem.AddPtrBuffer<byte>(
//		shaderVariable.Value.data(),
//		shaderVariable.Value.size(),
//		"ShaderVariable Value",  // tag instead of __FILE__ etc. if allowed
//		0,
//		"ShaderSystemDLL_ShaderVariableCPPToDLL"
//	);
//
//	if (variablePtr == nullptr)
//	{
//		return ShaderVariableDLL{};  // invalid
//	}
//
//	char* namePtr = nullptr;
//	size_t nameLen = shaderVariable.Name.size();
//
//	if (nameLen > 0)
//	{
//		const char* cstr = shaderVariable.Name.c_str();
//		namePtr = memorySystem.AddPtrBuffer<char>(
//			cstr,
//			nameLen + 1,
//			"ShaderVariable Name",
//			0,
//			"ShaderSystemDLL_ShaderVariableCPPToDLL"
//		);
//
//		if (namePtr == nullptr)
//		{
//			memorySystem.RemovePtrBuffer(variablePtr);
//			return ShaderVariableDLL{};
//		}
//
//		namePtr[nameLen] = '\0';  // ensure null-termination
//	}
//
//	return ShaderVariableDLL
//	{
//		.Name = namePtr,
//		.Size = shaderVariable.Size,
//		.ByteAlignment = shaderVariable.ByteAlignment,
//		.ValuePtr = variablePtr,
//		.ValueSize = shaderVariable.Value.size(),
//		.MemberTypeEnum = shaderVariable.MemberTypeEnum
//	};
//}
//
//VkPipelineShaderStageCreateInfo ShaderSystemDLL_LoadShader(const char* filename, VkShaderStageFlagBits shaderStages)
//{
//	return ShaderSystemDLL.LoadShader(filename, shaderStages);
//}
//
//ShaderPipelineDataDLL ShaderSystemDLL_LoadPipelineShaderData(StringListPtr pipelineShaderPaths, size_t listCount)
//{
//	Vector<String> pipelineShaderList;
//	pipelineShaderList.reserve(listCount);
//
//	for (size_t x = 0; x < listCount; ++x)
//	{
//		const char* cstr = pipelineShaderPaths[x];
//		pipelineShaderList.emplace_back(cstr ? cstr : "");
//	}
//	ShaderPipelineData shaderPipelineData = ShaderSystemDLL.LoadPipelineShaderData(pipelineShaderList);
//
//	Vector<const char*>* returnStringList;
//	returnStringList->reserve(shaderPipelineData.ShaderList.size());
//	for (const auto& str : shaderPipelineData.ShaderList)
//	{
//		returnStringList->emplace_back(str.c_str());
//	}
//
//	return ShaderPipelineDataDLL
//	{
//		.ShaderListPtr = returnStringList->data(),
//		.DescriptorBindingsListPtr = shaderPipelineData.DescriptorBindingsList.data(),
//		.ShaderStructListPtr = shaderPipelineData.ShaderStructList.data(),
//		.VertexInputBindingListPtr = shaderPipelineData.VertexInputBindingList.data(),
//		.VertexInputAttributeListPtr = shaderPipelineData.VertexInputAttributeList.data(),
//		.PushConstantListPtr = shaderPipelineData.PushConstantList.data(),
//		.ShaderListCount = returnStringList->size(),
//		.DescriptorBindingsListCount = shaderPipelineData.DescriptorBindingsList.size(),
//		.ShaderStructListCount = shaderPipelineData.ShaderStructList.size(),
//		.VertexInputBindingListCount = shaderPipelineData.VertexInputBindingList.size(),
//		.VertexInputAttributeListCount = shaderPipelineData.VertexInputAttributeList.size(),
//		.PushConstantListCount = shaderPipelineData.PushConstantList.size(),
//	};
//}
//
//void ShaderSystemDLL_LoadShaderPipelineStructPrototypes(StringListPtr shaderPathListPtr, size_t listCount)
//{
//	Vector<String> shaderPathList;
//	shaderPathList.reserve(listCount);
//
//	for (size_t x = 0; x < listCount; ++x)
//	{
//		const char* cstr = shaderPathListPtr[x];
//		shaderPathList.emplace_back(cstr ? cstr : "");
//	}
//	ShaderSystemDLL.LoadShaderPipelineStructPrototypes(shaderPathList);
//}
//
//SpvReflectSpecializationConstant** ShaderSystemDLL_LoadShaderSpecializationConstants(const SpvReflectShaderModule& module, size_t& outCount)
//{
//	return nullptr;
//}
//
//bool ShaderSystemDLL_CompileShaders(const char* fileDirectory, const char* outputDirectory)
//{
//	return ShaderSystemDLL.CompileShaders(fileDirectory, outputDirectory);
//}
//
//void ShaderSystemDLL_UpdatePushConstantBuffer(const char* pushConstantName)
//{
//	ShaderSystemDLL.UpdatePushConstantBuffer(pushConstantName);
//}
//
//void ShaderSystemDLL_UpdatePushConstantBuffer(ShaderPushConstantDLL& pushConstantStruct)
//{
//	Vector<byte> byteVector(static_cast<const byte*>(pushConstantStruct.PushConstantBufferPtr), static_cast<const byte*>(pushConstantStruct.PushConstantBufferPtr) + pushConstantStruct.PushConstantBufferSize);
//	ShaderPushConstant pushConstant = ShaderPushConstant
//	{
//		.PushConstantName = pushConstantStruct.PushConstantName,
//		.PushConstantSize = pushConstantStruct.PushConstantSize,
//		.ShaderStageFlags = pushConstantStruct.ShaderStageFlags,
//		.PushConstantVariableList = Vector<ShaderVariable>(pushConstantStruct.PushConstantVariableListPtr, pushConstantStruct.PushConstantVariableListPtr + pushConstantStruct.PushConstantVariableListCount),
//		.PushConstantBuffer = byteVector,
//		.GlobalPushContsant = pushConstantStruct.GlobalPushContsant
//	};
//	ShaderSystemDLL.UpdatePushConstantBuffer(pushConstant);
//}
//
//void ShaderSystemDLL_UpdateShaderBuffer(ShaderStructDLL& shaderStructDll, uint vulkanBufferId)
//{
//	Vector<ShaderVariable> shaderVariableList;
//	Vector<ShaderVariableDLL> shaderStructVariableDllList(shaderStructDll.ShaderBufferVariableListPtr, shaderStructDll.ShaderBufferVariableListPtr + shaderStructDll.ShaderBufferVariableListSize);
//	for (auto& variable : shaderStructVariableDllList)
//	{
//		shaderVariableList.emplace_back(ShaderSystemDLL_ShaderVariableDLLToCPP(variable));
//	}
//	Vector<byte> byteVector(static_cast<const byte*>(shaderStructDll.ShaderStructBufferPtr), static_cast<const byte*>(shaderStructDll.ShaderStructBufferPtr) + shaderStructDll.ShaderStructBufferSize);
//
//	ShaderStruct shaderStructCpp = ShaderStruct
//	{
//		.Name = shaderStructDll.Name,
//		.ShaderBufferSize = shaderStructDll.ShaderBufferSize,
//		.ShaderBufferVariableList = shaderVariableList,
//		.ShaderStructBufferId = shaderStructDll.ShaderStructBufferId,
//		.ShaderStructBuffer = byteVector
//	};
//	ShaderSystemDLL.UpdateShaderBuffer(shaderStructCpp, vulkanBufferId);
//}
//
//ShaderStructDLL ShaderSystemDLL_CopyShaderStructProtoType(const char* structName)
//{
//	ShaderStruct shaderStruct = ShaderSystemDLL.CopyShaderStructProtoType(structName);
//
//	Vector<ShaderVariableDLL> shaderVariableList;
//	Vector<ShaderVariableDLL> shaderStructVariableDllList(shaderStructDll.ShaderBufferVariableListPtr, shaderStructDll.ShaderBufferVariableListPtr + shaderStructDll.ShaderBufferVariableListSize);
//	return ShaderStructDLL
//	{
//		.Name = shaderStruct.Name.c_str(),
//		.ShaderBufferSize = shaderStruct.ShaderBufferSize,
//		.ShaderBufferVariableListPtr = shaderStruct.ShaderBufferVariableList.data(),
//		.ShaderBufferVariableListSize =  shaderStruct., 
//		.ShaderStructBufferId = shaderStruct.ShaderStructBufferId,
//		.ShaderStructBufferPtr = ,
//		.ShaderStructBufferSize
//	};
//}
//
//ShaderPipelineDataDLL ShaderSystemDLL_FindShaderModule(const char* shaderFile)
//{
//	return ShaderSystemDLL.FindShaderModule(shaderFile);
//}
//
//ShaderPushConstantDLL& ShaderSystemDLL_FindShaderPushConstant(const char* shaderFile)
//{
//	// TODO: insert return statement here
//}
//
//ShaderStructDLL ShaderSystemDLL_FindShaderProtoTypeStruct(const char* shaderKey)
//{
//	return ShaderStruct();
//}
//
//ShaderStructDLL& ShaderSystemDLL_FindShaderStruct(int vulkanBufferId)
//{
//	// TODO: insert return statement here
//}
//
//ShaderVariableDLL& ShaderSystemDLL_FindShaderPipelineStructVariable(ShaderStructDLL& shaderStruct, const char* variableName)
//{
//	// TODO: insert return statement here
//}
//
//ShaderVariableDLL& ShaderSystemDLL_FindShaderPushConstantStructVariable(ShaderPushConstantDLL& shaderPushConstant, const char* variableName)
//{
//	// TODO: insert return statement here
//}
//
//SpvReflectSpecializationConstant** ShaderSystemDLL_FindShaderSpecializationConstant(const SpvReflectSpecializationConstant** specializationConstantListPtr, size_t specializationConstantCount, const char* searchString, size_t& outCount)
//{
//	return nullptr;
//}
//
//bool ShaderSystemDLL_ShaderModuleExists(const char* shaderFile)
//{
//	return bool();
//}
//
//bool ShaderSystemDLL_ShaderPushConstantExists(const char* pushConstantName)
//{
//	return bool();
//}
//
//bool ShaderSystemDLL_ShaderStructPrototypeExists(const char* structKey)
//{
//	return bool();
//}
//
//bool ShaderSystemDLL_ShaderPipelineStructExists(uint vulkanBufferKey)
//{
//	return bool();
//}
//
//bool ShaderSystemDLL_SearchShaderConstantBufferExists(const ShaderPushConstantDLL* shaderPushConstantListPtr, size_t shaderPushConstantCount, const char* constBufferName)
//{
//	return bool();
//}
//
//bool ShaderSystemDLL_SearchShaderDescriptorBindingExists(const ShaderDescriptorBindingDLL* shaderDescriptorBindingListPtr, size_t shaderDescriptorBindingCount, const char* descriptorBindingName)
//{
//	return bool();
//}
//
//bool ShaderSystemDLL_SearchShaderPipelineStructExists(const ShaderStructDLL* shaderStructListPtr, size_t shaderStructCount, const char* structName)
//{
//	return bool();
//}
