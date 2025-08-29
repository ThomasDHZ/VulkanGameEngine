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
            for (auto& structVar : shaderStructList)
            {
                if (!ShaderStructPrototypeExists(structVar.Name))
                {
                    for (int x = 0; x < structVar.ShaderBufferVariableListCount; x++)
                    {
                        structVar.ShaderBufferVariableList[x].Value = memorySystem.AddPtrBuffer<byte>(structVar.ShaderBufferVariableListCount, __FILE__, __LINE__, __func__, structVar.Name.c_str());
                    }
                    PipelineShaderStructPrototypeMap[structVar.Name] = structVar;
                }
                else
                {
                    Shader_DestroyShaderStructData(structVar);
                }
            }
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

    for (auto& pushConstant : ShaderPushConstantMap)
    {
       
    
        memorySystem.RemovePtrBuffer<ShaderPushConstant>(&pushConstant.second);
    }
    for (auto& shaderStruct : PipelineShaderStructPrototypeMap)
    {
        memorySystem.RemovePtrBuffer<ShaderStruct>(&shaderStruct.second);
    }
    for (auto& shaderModule : PipelineShaderStructMap)
    {
        memorySystem.RemovePtrBuffer<ShaderStruct>(&shaderModule.second);
    }
    for (auto& pipelineData : ShaderModuleMap)
    {
        Shader_ShaderDestroy(pipelineData.second);
    }
}
