#pragma once
#include <Windows.h>
#include <dxcapi.h>
#include <wrl/client.h>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <vulkan/vulkan.h>
#include "TypeDef.h"
#include "MemorySystem.h"
#include "JsonStruct.h"

#ifdef __cplusplus
extern "C" {
#endif
    DLL_EXPORT void Shader_StartUp();
    DLL_EXPORT void Shader_LoadShaderPipelineStructPrototypes(const char** renderPassJsonList, size_t renderPassJsonCount);
    DLL_EXPORT const ShaderVariable* Shader_SearchGlobalShaderConstantVar(ShaderPushConstant* pushConstant, const char* varName);
    
    DLL_EXPORT ShaderPipelineData Shader_LoadPipelineShaderData(const char** pipelineShaderPaths, size_t pipelineShaderCount);
    DLL_EXPORT void Shader_UpdateShaderBuffer(const GraphicsRenderer& renderer, VulkanBuffer& vulkanBuffer, ShaderStruct* shaderStruct, size_t shaderCount);
    DLL_EXPORT void Shader_UpdatePushConstantBuffer(const GraphicsRenderer& renderer, ShaderPushConstant& pushConstantStruct);
    DLL_EXPORT ShaderStruct Shader_CopyShaderStructPrototype(const ShaderStruct& shaderStructToCopy);
    DLL_EXPORT ShaderVariable* Shader_SearchShaderConstStructVar(ShaderPushConstant* pushConstant, const char* varName);
    DLL_EXPORT ShaderVariable* Shader_SearchShaderStructVar(ShaderStruct* shaderStruct, const char* varName);
    DLL_EXPORT void Shader_ShaderDestroy(ShaderPipelineData& shader);
    DLL_EXPORT void Shader_DestroyShaderStructData(ShaderStruct* shaderStruct, size_t shaderStrucCount);
    DLL_EXPORT void Shader_DestroyPushConstantBufferData(ShaderPushConstant* pushConstant, size_t pushConstantCount);
    DLL_EXPORT void Shader_SetVariableDefaults(ShaderVariable& shaderVariable);
    DLL_EXPORT VkShaderModule Shader_BuildGLSLShaderFile(VkDevice device, const char* path);
    DLL_EXPORT bool Shader_BuildGLSLShaders(const char* command);
    DLL_EXPORT VkPipelineShaderStageCreateInfo Shader_CreateShader(VkShaderModule shaderModule, VkShaderStageFlagBits shaderStages);
#ifdef __cplusplus
}
#endif

DLL_EXPORT ShaderPushConstant* Shader_GetGlobalShaderPushConstant(const char* pushConstantName);
DLL_EXPORT ShaderStruct* Shader_LoadProtoTypeStructs(const char** pipelineShaderPaths, size_t pipelineShaderCount, size_t& outProtoTypeStructCount);
DLL_EXPORT void Shader_CompileShaders(VkDevice device, const char* fileDirectory, const char* outputDirectory);
DLL_EXPORT void Shader_UpdateGlobalShaderBuffer(const GraphicsRenderer& renderer, const String& pushConstantName);
DLL_EXPORT void Shader_UpdateShaderBuffer(const GraphicsRenderer& renderer, uint vulkanBufferId);
DLL_EXPORT bool Shader_ShaderModuleExists(const String& shaderFile);
DLL_EXPORT bool Shader_ShaderPushConstantExists(const String& pushConstantName);
DLL_EXPORT bool Shader_ShaderStructPrototypeExists(const String& structKey);
DLL_EXPORT bool Shader_ShaderStructExists(uint vulkanBufferKey);
DLL_EXPORT void Shader_Destroy();

DLL_EXPORT VkPipelineShaderStageCreateInfo Shader_LoadShader(VkDevice device, const char* path, VkShaderStageFlagBits shaderStages);
DLL_EXPORT ShaderPipelineData Shader_LoadShaderPipelineData(Vector<String> shaderPathList);
DLL_EXPORT ShaderPipelineData& Shader_FindShaderModule(const String& shaderFile);
DLL_EXPORT ShaderPushConstant& Shader_FindShaderPushConstant(const String& shaderFile);
DLL_EXPORT ShaderStruct Shader_FindShaderProtoTypeStruct(const String& structKey);
DLL_EXPORT ShaderStruct& Shader_FindShaderStruct(int vulkanBufferId);
DLL_EXPORT ShaderStruct Shader_CopyShaderStructProtoType(const String& structName);
DLL_EXPORT ShaderVariable* Shader_SearchShaderStruct(ShaderStruct& shaderStruct, const String& varName);

//ShaderDescriptorBinding* Shader_SearchDescriptorBindings(ShaderDescriptorBinding* shaderDescriptorBindingList, size_t shaderDescriptorBindingsCount, const char* descriptorBindingName);
ShaderStruct* Shader_SearchShaderStructs(ShaderStruct* shaderStructList, size_t shaderStructCount, const char* structName);
Vector<SpvReflectInterfaceVariable*> Shader_GetShaderVertexInputVariables(const SpvReflectShaderModule& module);
Vector<SpvReflectInterfaceVariable*> Shader_GetShaderVertexOutputVariables(const SpvReflectShaderModule& module);
Vector<SpvReflectSpecializationConstant*> Shader_GetShaderSpecializationConstant(const SpvReflectShaderModule& module);
Vector<SpvReflectSpecializationConstant*> Shader_SearchShaderSpecializationConstant(Vector<SpvReflectSpecializationConstant*>& specializationConstantList, const char* searchString);
ShaderStruct Shader_GetShaderStruct(SpvReflectTypeDescription& shaderInfo);
void Shader_DestroyShaderBindingData(ShaderDescriptorBinding* descriptorBinding, size_t descriptorCount);
bool Shader_SearchShaderConstBufferExists(const Vector<ShaderPushConstant>& shaderPushConstantList, const String& constBufferName);
bool Shader_SearchDescriptorBindingExists(const Vector<ShaderDescriptorBinding>& shaderDescriptorBindingList, const String& descriptorBindingName);
bool Shader_SearchShaderStructExists(ShaderStruct* shaderStructList, size_t shaderStructCount, const char* structName);
bool Shader_SearchShaderStructVarExists(const ShaderStruct* shaderStruct, const char* varName);
void Shader_GetShaderInputVertexVariables(const SpvReflectShaderModule& module, Vector<VkVertexInputBindingDescription>& vertexInputBindingList, Vector<VkVertexInputAttributeDescription>& vertexInputAttributeList);
void Shader_GetShaderConstBuffer(const SpvReflectShaderModule& module, Vector<ShaderPushConstant>& shaderPushConstantList);
void Shader_GetShaderDescriptorBindings(const SpvReflectShaderModule& module, Vector<ShaderDescriptorBinding>& shaderDescriptorBindingList);
void Shader_GetShaderDescriptorSetInfo(const SpvReflectShaderModule& module, Vector<ShaderStruct>& shaderStruct);

VkShaderModule Shader_ReadGLSLShader(VkDevice device, const char* path, VkShaderStageFlagBits stage);
void Shader_CompileGLSLShaders(VkDevice device, const char* fileDirectory, const char* outputDirectory);
Microsoft::WRL::ComPtr<IDxcBlob> Shader_CompileHLSLShaders(VkDevice device, const String& path, Microsoft::WRL::ComPtr<IDxcCompiler3>& dxc_compiler, Microsoft::WRL::ComPtr<IDxcIncludeHandler>& defaultIncludeHandler, VkShaderStageFlagBits stage);

LPWSTR Shader_StringToLPWSTR(const String& str);
String Shader_ConvertLPCWSTRToString(LPCWSTR lpcwszStr);
void Shader_uint32ToUnsignedCharString(uint32 value, String& string);
const char* Renderer_GetShaderReflectError(SpvReflectResult result);

#define SPV_VULKAN_RESULT(call) { \
    SpvReflectResult result = (call); \
    if (result != SPV_REFLECT_RESULT_SUCCESS) { \
        fprintf(stderr, "Error in %s at %s:%d (%s): %s\n", \
                #call, __FILE__, __LINE__, __func__, Renderer_GetShaderReflectError(result)); \
    } \
}

class ShaderSystem
{
private:
	Microsoft::WRL::ComPtr<IDxcUtils> dxc_utils;
	Microsoft::WRL::ComPtr<IDxcCompiler3> dxc_compiler;
	Microsoft::WRL::ComPtr<IDxcIncludeHandler> DefaultIncludeHandler;

public:
    UnorderedMap<String, ShaderPipelineData> ShaderModuleMap;
	UnorderedMap<String, ShaderPushConstant> ShaderPushConstantMap;
	UnorderedMap<String, ShaderStruct>  PipelineShaderStructPrototypeMap;
	UnorderedMap<int, ShaderStruct>  PipelineShaderStructMap;

    ShaderSystem() { }
    ~ShaderSystem() { }

    void StartUp() { Shader_StartUp(); }
    ShaderPipelineData LoadShaderPipelineData(Vector<String> shaderPathList) { return Shader_LoadShaderPipelineData(shaderPathList); }
    const ShaderVariable* SearchGlobalShaderConstantVar(ShaderPushConstant* pushConstant, const char* varName) { return Shader_SearchShaderConstStructVar(pushConstant, varName); }
    ShaderVariable* SearchShaderStruct(ShaderStruct& shaderStruct, const String& varName) { return Shader_SearchShaderStructVar(&shaderStruct, varName.c_str()); }
    void UpdateGlobalShaderBuffer(const String& pushConstantName) { Shader_UpdateGlobalShaderBuffer(renderer, pushConstantName); }
    void UpdateShaderBuffer(uint vulkanBufferId) { Shader_UpdateShaderBuffer(renderer, vulkanBufferId); }
    ShaderPushConstant* GetGlobalShaderPushConstant(const String& pushConstantName) { return Shader_GetGlobalShaderPushConstant(pushConstantName.c_str()); }
    void LoadShaderPipelineStructPrototypes(const Vector<String>& renderPassJsonList)
    {
        Vector<const char*> c_strs;
        c_strs.reserve(renderPassJsonList.size());

        for (const auto& str : renderPassJsonList)
        {
            c_strs.push_back(str.c_str());
        }

        const char** cStringArray = c_strs.data();
        Shader_LoadShaderPipelineStructPrototypes(cStringArray, renderPassJsonList.size());
    }
    ShaderPipelineData& FindShaderModule(const String& shaderFile) { return Shader_FindShaderModule(shaderFile); }
    ShaderPushConstant& FindShaderPushConstant(const String& shaderFile) { return Shader_FindShaderPushConstant(shaderFile); }
    ShaderStruct FindShaderProtoTypeStruct(const String& shaderKey) { return Shader_FindShaderProtoTypeStruct(shaderKey); }
    ShaderStruct& FindShaderStruct(int vulkanBufferId) { return Shader_FindShaderStruct(vulkanBufferId); }
    ShaderStruct CopyShaderStructProtoType(const String& structName) { ShaderStruct shaderStructCopy = FindShaderProtoTypeStruct(structName); return Shader_CopyShaderStructPrototype(shaderStructCopy); }
    void CompileShaders(const char* shaderFilePath, const char* shaderOutPutPirectory) { Shader_CompileShaders(renderer.Device, shaderFilePath, shaderOutPutPirectory); }
    const bool ShaderPushConstantExists(const String& pushConstantName) const { return Shader_ShaderPushConstantExists(pushConstantName); }
    const bool ShaderModuleExists(const String& shaderFile) const { return Shader_ShaderModuleExists(shaderFile); }
    const bool ShaderStructPrototypeExists(const String& structKey) const { return Shader_ShaderStructPrototypeExists(structKey); }
    const bool ShaderStructExists(uint vulkanBufferKey) const { return Shader_ShaderStructExists(vulkanBufferKey); }
    void Destroy() { Shader_Destroy(); }
};
DLL_EXPORT ShaderSystem shaderSystem;