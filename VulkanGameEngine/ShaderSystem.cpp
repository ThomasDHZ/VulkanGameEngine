#include "ShaderSystem.h"
#include "RenderSystem.h"
#include "MeshSystem.h"
#include "TextureSystem.h"
#include "BufferSystem.h"
#include <algorithm>
#include <iostream>
#include <string_view>

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

VkPipelineShaderStageCreateInfo ShaderSystem::CreateShader(VkDevice device, const String& path, VkShaderStageFlagBits shaderStages)
{
    return Shader_CreateShader(device, path, shaderStages);
}

ShaderPiplineData ShaderSystem::AddShaderModule(Vector<String> shaderPathList)
{
    ShaderPiplineData pipelineData = Shader_GetShaderData(shaderPathList.data(), shaderPathList.size());
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
                .PushConstantVariableList = memorySystem.AddPtrBuffer<ShaderVariable>(pushConstant.PushConstantVariableList, pushConstant.PushConstantVariableListCount, __FILE__, __LINE__, __func__, pushConstant.PushConstantName.c_str()),
                .PushConstantBuffer = memorySystem.AddPtrBuffer<byte>(pushConstant.PushConstantSize, __FILE__, __LINE__, __func__, pushConstant.PushConstantName.c_str()),
                .GlobalPushContant = pushConstant.GlobalPushContant
            };

            for (int x = 0; x < ShaderPushConstantMap[pushConstant.PushConstantName].PushConstantVariableListCount; x++)
            {
                ShaderVariable* variablePtr = &pushConstant.PushConstantVariableList[x];
                ShaderPushConstantMap[pushConstant.PushConstantName].PushConstantVariableList[x].Value = memorySystem.AddPtrBuffer<byte>(ShaderPushConstantMap[pushConstant.PushConstantName].PushConstantVariableList[x].Size, __FILE__, __LINE__, __func__, ShaderPushConstantMap[pushConstant.PushConstantName].PushConstantVariableList[x].Name.c_str());
                Shader_SetVariableDefaults(ShaderPushConstantMap[pushConstant.PushConstantName].PushConstantVariableList[x]);
                memorySystem.RemovePtrBuffer<ShaderVariable>(variablePtr);
            }
        }
        memorySystem.RemovePtrBuffer<ShaderVariable>(pushConstant.PushConstantVariableList);
        memorySystem.RemovePtrBuffer(pushConstant.PushConstantBuffer);
    }
    ShaderModuleMap[pipelineData.ShaderList[0]] = pipelineData;
    return pipelineData;
}

ShaderVariable* ShaderSystem::SearchGlobalShaderConstantVar(ShaderPushConstant* pushConstant, const String& varName)
{
    if (pushConstant == nullptr)
    {
        return nullptr;
    }

    auto it = std::find_if(pushConstant->PushConstantVariableList, pushConstant->PushConstantVariableList + pushConstant->PushConstantVariableListCount,
        [&](const ShaderVariable& var) {
            return var.Name == varName;
        }
    );

    if (it != pushConstant->PushConstantVariableList + pushConstant->PushConstantVariableListCount) {
        ShaderVariable& foundVar = *it;
    }

    return it;  
}

ShaderVariable* ShaderSystem::SearchShaderStruct(ShaderStruct& shaderStruct, const String& varName)
{
    return Shader_SearchShaderStructhVar(shaderStruct, varName);
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
    if (ShaderPushConstantExists(pushConstantName))
    {
        return &ShaderPushConstantMap[pushConstantName];
    }
    return nullptr;
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
            ShaderStruct* shaderStructArray = Shader_LoadProtoTypeStructs(shaderJsonList.data(), shaderJsonList.size(), protoTypeStructCount);

            Span<ShaderStruct> shaderStructList(shaderStructArray, protoTypeStructCount);
            for (auto& shaderStruct : shaderStructList)
            {
                if (!ShaderStructPrototypeExists(shaderStruct.Name))
                {
                    PipelineShaderStructPrototypeMap[shaderStruct.Name] = ShaderStruct
                    {
                        .Name = shaderStruct.Name,
                        .ShaderBufferSize = shaderStruct.ShaderBufferSize,
                        .ShaderBufferVariableListCount = shaderStruct.ShaderBufferVariableListCount,
                        .ShaderBufferVariableList = memorySystem.AddPtrBuffer<ShaderVariable>(shaderStruct.ShaderBufferVariableList, shaderStruct.ShaderBufferVariableListCount, __FILE__, __LINE__, __func__, shaderStruct.Name.c_str()),
                        .ShaderStructBufferId = shaderStruct.ShaderStructBufferId,
                        .ShaderStructBuffer = memorySystem.AddPtrBuffer<byte>(shaderStruct.ShaderBufferSize, __FILE__, __LINE__, __func__, shaderStruct.Name.c_str())
                    };
                }
                memorySystem.RemovePtrBuffer(shaderStruct.ShaderBufferVariableList);
            }
            memorySystem.RemovePtrBuffer<ShaderStruct>(shaderStructArray);
        }
    }
}

ShaderPiplineData& ShaderSystem::FindShaderModule(const String& shaderFile)
{
    auto it = ShaderModuleMap.find(shaderFile);
    if (it != ShaderModuleMap.end())
    {
        return it->second;
    }
    throw std::out_of_range("ShaderModuleMap not found for given shaderFile");
}

ShaderPushConstant& ShaderSystem::FindShaderPushConstant(const String& shaderFile)
{
    auto it = ShaderPushConstantMap.find(shaderFile);
    if (it != ShaderPushConstantMap.end())
    {
        return it->second;
    }
    throw std::out_of_range("ShaderPushConstantSourceMap not found for given shader push constant");
}

ShaderStruct ShaderSystem::FindShaderProtoTypeStruct(const String& shaderKey)
{
    auto it = PipelineShaderStructPrototypeMap.find(shaderKey);
    if (it != PipelineShaderStructPrototypeMap.end())
    {
        return it->second;
    }
    throw std::out_of_range("PipelineShaderStructPrototypeMap not found");
}

ShaderStruct& ShaderSystem::FindShaderStruct(int vulkanBufferId)
{
    auto it = PipelineShaderStructMap.find(vulkanBufferId);
    if (it != PipelineShaderStructMap.end())
    {
        return it->second;
    }
    throw std::out_of_range("PipelineShaderStructMap not found");
}

ShaderStruct ShaderSystem::CopyShaderStructProtoType(const String& structName)
{
    ShaderStruct shaderStructCopy = FindShaderProtoTypeStruct(structName);
    return Shader_CopyShaderStructPrototype(shaderStructCopy);
}

bool ShaderSystem::ShaderPushConstantExists(const String& pushConstantName)
{
    auto it = ShaderPushConstantMap.find(pushConstantName);
    if (it != ShaderPushConstantMap.end())
    {
        return true;
    }
    return false;
}

bool ShaderSystem::ShaderModuleExists(const String& shaderFile)
{
    auto it = ShaderModuleMap.find(shaderFile);
    if (it != ShaderModuleMap.end())
    {
        return true;
    }
    return false;
}

bool ShaderSystem::ShaderStructPrototypeExists(const String& structKey)
{
    auto it = PipelineShaderStructPrototypeMap.find(structKey);
    if (it != PipelineShaderStructPrototypeMap.end())
    {
        return true;
    }
    return false;
}

bool ShaderSystem::ShaderStructExists(uint vulkanBufferKey)
{
    auto it = PipelineShaderStructMap.find(vulkanBufferKey);
    if (it != PipelineShaderStructMap.end())
    {
        return true;
    }
    return false;
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
