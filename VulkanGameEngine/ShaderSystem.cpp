#include "ShaderSystem.h"
#include "RenderSystem.h"
#include "MeshSystem.h"
#include "TextureSystem.h"
#include "BufferSystem.h"
#include <algorithm>
#include <iostream>
#include <string_view>
#include <CHelper.h>

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
    Vector<String> pushConstantKeys;
    for (const auto& pair : ShaderPushConstantMap)
    {
        pushConstantKeys.push_back(pair.first);
    }
    for (const auto& key : pushConstantKeys)
    {
        auto& pushConstant = ShaderPushConstantMap[key];
        Shader_DestroyPushConstantBufferData(&pushConstant);
    }
    ShaderPushConstantMap.clear();

    Vector<String> shaderStructProtoKeys;
    for (const auto& pair : PipelineShaderStructPrototypeMap)
    {
        shaderStructProtoKeys.push_back(pair.first);
    }
    for (const auto& key : shaderStructProtoKeys)
    {
        auto& shaderStruct = PipelineShaderStructPrototypeMap[key];
        Shader_DestroyShaderStructData(&shaderStruct);
    }
    PipelineShaderStructPrototypeMap.clear();

    Vector<int> shaderStructKeys;
    for (const auto& pair : PipelineShaderStructMap)
    {
        shaderStructKeys.push_back(pair.first);
    }
    for (const auto& key : shaderStructKeys)
    {
        auto& shaderStruct = PipelineShaderStructMap[key];
        Shader_DestroyShaderStructData(&shaderStruct);
    }
    PipelineShaderStructMap.clear();

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

VkPipelineShaderStageCreateInfo ShaderSystem::CreateShader(VkDevice device, const String& path, VkShaderStageFlagBits shaderStages)
{
    return Shader_CreateShader(device, path.c_str(), shaderStages);
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
            ShaderPushConstantMap[pushConstant.PushConstantName] = ShaderPushConstant
            {
                .PushConstantName = pushConstant.PushConstantName,
                .PushConstantSize = pushConstant.PushConstantSize,
                .PushConstantVariableListCount = pushConstant.PushConstantVariableListCount,
                .ShaderStageFlags = pushConstant.ShaderStageFlags,
                .PushConstantVariableList = memorySystem.AddPtrBuffer<ShaderVariable>(pushConstant.PushConstantVariableList, pushConstant.PushConstantVariableListCount, __FILE__, __LINE__, __func__, pushConstant.PushConstantName),
                .PushConstantBuffer = memorySystem.AddPtrBuffer<byte>(pushConstant.PushConstantSize, __FILE__, __LINE__, __func__, pushConstant.PushConstantName),
                .GlobalPushContant = pushConstant.GlobalPushContant
            };

            for (int x = 0; x < ShaderPushConstantMap[pushConstant.PushConstantName].PushConstantVariableListCount; x++)
            {
                ShaderVariable* variablePtr = &pushConstant.PushConstantVariableList[x];
                ShaderPushConstantMap[pushConstant.PushConstantName].PushConstantVariableList[x].Value = memorySystem.AddPtrBuffer<byte>(ShaderPushConstantMap[pushConstant.PushConstantName].PushConstantVariableList[x].Size, __FILE__, __LINE__, __func__, ShaderPushConstantMap[pushConstant.PushConstantName].PushConstantVariableList[x].Name);
                Shader_SetVariableDefaults(ShaderPushConstantMap[pushConstant.PushConstantName].PushConstantVariableList[x]);
                memorySystem.RemovePtrBuffer<ShaderVariable>(variablePtr);
            }
        }
        memorySystem.RemovePtrBuffer<ShaderVariable>(pushConstant.PushConstantVariableList);
        memorySystem.RemovePtrBuffer(pushConstant.PushConstantBuffer);
    }
    CHelper_DestroyConstCharPtrPtr(cShaderList);
    ShaderModuleMap[pipelineData.ShaderList[0]] = pipelineData;
    return pipelineData;
}

ShaderVariable* ShaderSystem::SearchGlobalShaderConstantVar(ShaderPushConstant* pushConstant, const char* varName)
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
                    const char* copiedStr = memorySystem.AddPtrBuffer(shaderStruct.Name, __FILE__, __LINE__, __func__, "shaderStructToCopy.Name copy");
                    PipelineShaderStructPrototypeMap[copiedStr] = ShaderStruct
                    {
                        .Name = copiedStr,
                        .ShaderBufferSize = shaderStruct.ShaderBufferSize,
                        .ShaderBufferVariableListCount = shaderStruct.ShaderBufferVariableListCount,
                        .ShaderBufferVariableList = memorySystem.AddPtrBuffer<ShaderVariable>(shaderStruct.ShaderBufferVariableList, shaderStruct.ShaderBufferVariableListCount, __FILE__, __LINE__, __func__, copiedStr),
                        .ShaderStructBufferId = shaderStruct.ShaderStructBufferId,
                        .ShaderStructBuffer = memorySystem.AddPtrBuffer<byte>(shaderStruct.ShaderBufferSize, __FILE__, __LINE__, __func__, copiedStr)
                    };
                    //memorySystem.RemovePtrBuffer<const char>(copiedStr);
                }
                memorySystem.RemovePtrBuffer(shaderStruct.ShaderBufferVariableList);
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