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
            pushConstant.PushConstantBuffer = memorySystem.AddPtrBuffer<byte>(pushConstant.PushConstantSize, __FILE__, __LINE__, __func__, pushConstant.PushConstantName.c_str());
            for (int x = 0; x < pushConstant.PushConstantVariableListCount; x++)
            {
                pushConstant.PushConstantVariableList[x].Value = memorySystem.AddPtrBuffer<byte>(pushConstant.PushConstantVariableList[x].Size, __FILE__, __LINE__, __func__, pushConstant.PushConstantVariableList[x].Name.c_str());
            }
            ShaderPushConstantMap[pushConstant.PushConstantName] = pushConstant;
        }
        else
        {
            Shader_DestroyPushConstantBufferVariableData(pushConstant);
        }
    }
    return pipelineData;
}

ShaderVariable* ShaderSystem::SearchGlobalShaderConstantVar(ShaderPushConstant& pushConstant, const String& varName)
{
    auto it = std::find_if(pushConstant.PushConstantVariableList, pushConstant.PushConstantVariableList + pushConstant.PushConstantVariableListCount,
        [&](const ShaderVariable& var) {
            return var.Name == varName;
        }
    );

    if (it != pushConstant.PushConstantVariableList + pushConstant.PushConstantVariableListCount) {
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

    const ShaderPushConstant& pushConstant = ShaderPushConstantMap[pushConstantName];
    if (pushConstant.PushConstantBuffer == nullptr)
    {
        std::cerr << "Error: PushConstantBuffer is null for push constant '" << pushConstantName << "'!" << std::endl;
        return;
    }

    size_t offset = 0;
    Span<ShaderVariable> pushConstantVarList(pushConstant.PushConstantVariableList, pushConstant.PushConstantVariableListCount);
    for (const auto& pushConstantVar : pushConstantVarList)
    {
        if (pushConstantVar.Value == nullptr)
        {
            std::cerr << "Warning: Value pointer for variable '" << pushConstantVar.Name << "' is null!" << std::endl;
            continue;
        }

        offset = (offset + pushConstantVar.ByteAlignment - 1) & ~(pushConstantVar.ByteAlignment - 1);
        void* dest = static_cast<byte*>(pushConstant.PushConstantBuffer) + offset;
        if (pushConstantVar.Name == "MeshBufferIndex")
        {
            uint32_t sdf = 0;
            memcpy(dest, &sdf, sizeof(uint32_t));
        }
        else
        {
            memcpy(dest, pushConstantVar.Value, pushConstantVar.Size);
        }
        offset += pushConstantVar.Size;
    }
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
            Vector<String> shaderJsonList = Vector<String> { pipelineJson["ShaderList"][0], pipelineJson["ShaderList"][1] };
            ShaderStruct* shaderStructArray = Shader_LoadProtoTypeStructs(shaderJsonList.data(), shaderJsonList.size(), protoTypeStructCount);

            Span<ShaderStruct> shaderStructList(shaderStructArray, protoTypeStructCount);
            for (auto& structStruct : shaderStructList)
            {
                if (!ShaderStructPrototypeExists(structStruct.Name))
                {
                    PipelineShaderStructPrototypeMap[structStruct.Name] = structStruct;
                }
                else
                {
                    Shader_DestroyShaderStructData(structStruct);
                }
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
    ShaderStruct shaderStruct = FindShaderProtoTypeStruct(structName);
    shaderStruct.ShaderStructBuffer = memorySystem.AddPtrBuffer<byte>(shaderStruct.ShaderBufferSize, __FILE__, __LINE__, __func__, shaderStruct.Name.c_str());
    Span<ShaderVariable> structVariableList(shaderStruct.ShaderBufferVariableList, shaderStruct.ShaderBufferVariableListCount);
    for (auto& shaderVar : structVariableList)
    {
        shaderVar.Value = memorySystem.AddPtrBuffer<byte>(shaderVar.Size, __FILE__, __LINE__, __func__, shaderVar.Name.c_str());
    }
    return shaderStruct;
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
    for (auto& shaderStruct : PipelineShaderStructPrototypeMap)
    {
        Shader_DestroyShaderStructData(shaderStruct.second);
    }
    for (auto& shaderStruct : PipelineShaderStructPrototypeMap)
    {
        ShaderStruct* ptr = &shaderStruct.second;
        memorySystem.RemovePtrBuffer(ptr);
    }
    //for (auto& shaderStruct : PipelineShaderStructMap)
    //{
    //    Shader_DestroyShaderStructData(shaderStruct.second);
    //}
    //for (auto& shaderStruct : PipelineShaderStructMap)
    //{
    //    ShaderStruct* ptr = &shaderStruct.second;
    //    memorySystem.RemovePtrBuffer(ptr);
    //}
    for (auto& pipelineData : ShaderModuleMap)
    {
        Shader_ShaderDestroy(pipelineData.second);
    }
}
