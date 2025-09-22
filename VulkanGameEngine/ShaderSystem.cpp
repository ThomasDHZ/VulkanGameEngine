#include "ShaderSystem.h"
#include "RenderSystem.h"
#include "MeshSystem.h"
#include "TextureSystem.h"
#include "BufferSystem.h"
#include <algorithm>
#include <iostream>
#include <string_view>
#include <CHelper.h>
#include "EngineConfigSystem.h"

ShaderSystem shaderSystem = ShaderSystem();

ShaderSystem::ShaderSystem()
{
}

ShaderSystem::~ShaderSystem()
{
}

void ShaderSystem::StartUp()
{
    Shader_StartUp();
}

void ShaderSystem::Destroy()
{
    Vector<ShaderPushConstant> shaderPushConstantList;
    shaderPushConstantList.reserve(ShaderPushConstantMap.size());
    for (const auto& pair : ShaderPushConstantMap) {
        shaderPushConstantList.push_back(pair.second);
    }
    Shader_DestroyPushConstantBufferData(shaderPushConstantList.data(), shaderPushConstantList.size());

    Vector<ShaderStruct> shaderStructProtoList;
    shaderStructProtoList.reserve(PipelineShaderStructPrototypeMap.size());
    for (const auto& pair : PipelineShaderStructPrototypeMap) {
        shaderStructProtoList.push_back(pair.second);
    }
    Shader_DestroyShaderStructData(shaderStructProtoList.data(), shaderStructProtoList.size());

    Vector<ShaderStruct> shaderStructList;
    shaderStructList.reserve(PipelineShaderStructMap.size());
    for (const auto& pair : PipelineShaderStructMap) {
        shaderStructList.push_back(pair.second);
    }
    Shader_DestroyShaderStructData(shaderStructList.data(), shaderStructList.size());

    Vector<String> shaderModuleKeys;
    for (const auto& pair : ShaderModuleMap)
    {
        shaderModuleKeys.push_back(pair.first);
    }
    for (const auto& key : shaderModuleKeys)
    {
        auto& pipelineData = ShaderModuleMap[key];
        Shader_ShaderDestroy(pipelineData);
    }

    ShaderModuleMap.clear();
}

ShaderPipelineData ShaderSystem::LoadShaderPipelineData(Vector<String> shaderPathList)
{
    const char** cShaderList = CHelper_VectorToConstCharPtrPtr(shaderPathList);
    ShaderPipelineData pipelineData = Shader_LoadPipelineShaderData(cShaderList, shaderPathList.size());
    Span<ShaderPushConstant> pushConstantList(pipelineData.PushConstantList, pipelineData.PushConstantCount);
    for (auto& pushConstant : pushConstantList)
    {
        if (!ShaderPushConstantExists(pushConstant.PushConstantName))
        {
            ShaderPushConstant shaderPushConstant = ShaderPushConstant
            {
                .PushConstantName = pushConstant.PushConstantName,
                .PushConstantSize = pushConstant.PushConstantSize,
                .PushConstantVariableList = pushConstant.PushConstantVariableList,
                .PushConstantBuffer = memorySystem.AddPtrBuffer<byte>(pushConstant.PushConstantSize, __FILE__, __LINE__, __func__, pushConstant.PushConstantName.c_str()),
                .ShaderStageFlags = pushConstant.ShaderStageFlags,
                .GlobalPushContant = pushConstant.GlobalPushContant
            };

            for (auto& shaderVariable : shaderPushConstant.PushConstantVariableList)
            {
                const String variableName = shaderVariable.Name;
                ShaderVariable* shaderVariablePtr = &shaderVariable;
                shaderVariable.Value = memorySystem.AddPtrBuffer<byte>(shaderVariable.Size, __FILE__, __LINE__, __func__, variableName.c_str());
                Shader_SetVariableDefaults(shaderVariable.Value, shaderVariable.MemberTypeEnum);
                memorySystem.RemovePtrBuffer<ShaderVariable>(shaderVariablePtr);
            }
            ShaderPushConstantMap[pushConstant.PushConstantName] = shaderPushConstant;
        }
        memorySystem.RemovePtrBuffer(pushConstant.PushConstantBuffer);
    }
    CHelper_DestroyConstCharPtrPtr(cShaderList);
    ShaderModuleMap[pipelineData.ShaderList[0]] = pipelineData;
    return pipelineData;
}

const ShaderVariable* ShaderSystem::SearchGlobalShaderConstantVar(ShaderPushConstant* pushConstant, const char* varName)
{
    return Shader_SearchShaderConstStructVar(pushConstant, varName);
}

ShaderVariable* ShaderSystem::SearchShaderStruct(ShaderStruct& shaderStruct, const String& varName)
{
    return Shader_SearchShaderStructVar(&shaderStruct, varName.c_str());
}

void ShaderSystem::UpdateGlobalShaderBuffer(const String& pushConstantName)
{
    if (!ShaderPushConstantExists(pushConstantName))
    {
        std::cerr << "Error: Push constant '" << pushConstantName << "' does not exist!" << std::endl;
        return;
    }
    Shader_UpdatePushConstantBuffer(renderSystem.renderer, ShaderPushConstantMap[pushConstantName]);
}

void ShaderSystem::UpdateShaderBuffer(uint vulkanBufferId)
{
    if (!ShaderStructExists(vulkanBufferId))
    {
        return;
    }

    ShaderStruct& shaderStruct = PipelineShaderStructMap[vulkanBufferId];
    VulkanBuffer& vulkanBuffer = bufferSystem.FindVulkanBuffer(vulkanBufferId);
    Shader_UpdateShaderBuffer(renderSystem.renderer, vulkanBuffer, &shaderStruct, 1);
}

ShaderPushConstant* ShaderSystem::GetGlobalShaderPushConstant(const String& pushConstantName)
{
    return ShaderPushConstantExists(pushConstantName) ? &ShaderPushConstantMap[pushConstantName] : nullptr;
}

void ShaderSystem::LoadShaderPipelineStructPrototypes(const Vector<String>& renderPassJsonList)
{
    size_t protoTypeStructCount = 0;
    for (int x = 0; x < renderPassJsonList.size(); x++)
    {
        nlohmann::json renderPassJson = Json::ReadJson(renderPassJsonList[x]);
        for (int x = 0; x < renderPassJson["RenderPipelineList"].size(); x++)
        {
            nlohmann::json pipelineJson = Json::ReadJson(renderPassJson["RenderPipelineList"][x]);
            Vector<String> shaderJsonList = Vector<String>{ pipelineJson["ShaderList"][0], pipelineJson["ShaderList"][1] };
            const char** cShaderList = CHelper_VectorToConstCharPtrPtr(shaderJsonList);
            ShaderStruct* shaderStructArray = Shader_LoadProtoTypeStructs(cShaderList, shaderJsonList.size(), protoTypeStructCount);
            Span<ShaderStruct> shaderStructList(shaderStructArray, protoTypeStructCount);
            for (auto& shaderStruct : shaderStructList)
            {
                if (!ShaderStructPrototypeExists(shaderStruct.Name))
                {
                    PipelineShaderStructPrototypeMap[shaderStruct.Name] = ShaderStruct
                    {
                        .Name = shaderStruct.Name,
                        .ShaderBufferSize = shaderStruct.ShaderBufferSize,
                        .ShaderBufferVariableList = shaderStruct.ShaderBufferVariableList,
                        .ShaderStructBufferId = shaderStruct.ShaderStructBufferId,
                        .ShaderStructBuffer = memorySystem.AddPtrBuffer<byte>(shaderStruct.ShaderBufferSize, __FILE__, __LINE__, __func__, shaderStruct.Name.c_str())
                    };
                }
            }
            memorySystem.RemovePtrBuffer<ShaderStruct>(shaderStructArray);
            CHelper_DestroyConstCharPtrPtr(cShaderList);
        }
    }
}

ShaderPipelineData& ShaderSystem::FindShaderModule(const String& shaderFile)
{
    return ShaderModuleMap.at(shaderFile);
}

ShaderPushConstant& ShaderSystem::FindShaderPushConstant(const String& shaderFile)
{
    return ShaderPushConstantMap.at(shaderFile);
}

ShaderStruct ShaderSystem::FindShaderProtoTypeStruct(const String& shaderKey)
{
    return PipelineShaderStructPrototypeMap.at(shaderKey);
}

ShaderStruct& ShaderSystem::FindShaderStruct(int vulkanBufferId)
{
    return PipelineShaderStructMap.at(vulkanBufferId);
}

ShaderStruct ShaderSystem::CopyShaderStructProtoType(const String& structName)
{
    ShaderStruct shaderStructCopy = FindShaderProtoTypeStruct(structName);
    return Shader_CopyShaderStructPrototype(shaderStructCopy);
}

const bool ShaderSystem::ShaderPushConstantExists(const String& pushConstantName) const
{
    return ShaderPushConstantMap.contains(pushConstantName);
}

const bool ShaderSystem::ShaderModuleExists(const String& shaderFile) const
{
    return ShaderModuleMap.contains(shaderFile);
}

const bool ShaderSystem::ShaderStructPrototypeExists(const String& structKey) const
{
    return PipelineShaderStructPrototypeMap.contains(structKey);
}

const bool ShaderSystem::ShaderStructExists(uint vulkanBufferKey) const
{
    return PipelineShaderStructMap.contains(vulkanBufferKey);
}

void ShaderSystem::CompileShaders(const char* shaderFilePath)
{
     Shader_CompileShaders(renderSystem.renderer.Device, shaderFilePath, configSystem.CompiledShaderOutputDirectory.c_str());
}