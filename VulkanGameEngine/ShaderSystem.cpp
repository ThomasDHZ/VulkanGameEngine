#include "ShaderSystem.h"
#include "RenderSystem.h"
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

void ShaderSystem::VertexDataFromSpirv(const String& path)
{
    FileState file = File_Read(path.c_str());
    Vector<byte> shaderCode = Vector<byte>(file.Data, file.Data + file.Size);
    //Shader_VertexDataFromSpirv(shaderCode);
}

VkPipelineShaderStageCreateInfo ShaderSystem::CreateShader(VkDevice device, const String& path, VkShaderStageFlagBits shaderStages)
{
    return Shader_CreateShader(device, path, shaderStages);
}

void ShaderSystem::AddShaderModule(const String& modulePath)
{
    const char* fileName = File_GetFileNameFromPath(modulePath.c_str());
    if (!ShaderModuleExists(fileName))
    {
        ShaderModuleMap[fileName] = Shader_GetShaderData(modulePath);

        Vector<ShaderPushConstant> pushConstantList = Shader_GetShaderConstBuffer(ShaderModuleMap[fileName]);
        for(auto& pushConstant : pushConstantList)
        {
            if (!ShaderPushConstantExists(pushConstant.StructName) &&
                !ShaderPushConstantExists(pushConstant.PushConstantName))
            {
                if (!pushConstant.StructName.empty())
                {
                    ShaderPushConstantSourceMap[pushConstant.StructName] = pushConstant;
                    GlobalPushContantShaderPushConstantMap[pushConstant.StructName] = pushConstant;
                    GlobalPushContantShaderPushConstantMap[pushConstant.PushConstantName].PushConstantBuffer = memorySystem.AddPtrBuffer<byte>(pushConstant.PushConstantSize, __FILE__, __LINE__, __func__);

                    Vector<ShaderVariable> variableList = Vector<ShaderVariable>(GlobalPushContantShaderPushConstantMap[pushConstant.PushConstantName].PushConstantVariableList, GlobalPushContantShaderPushConstantMap[pushConstant.PushConstantName].PushConstantVariableList + GlobalPushContantShaderPushConstantMap[pushConstant.PushConstantName].PushConstantVariableListCount);
                    for (int x = 0; x < GlobalPushContantShaderPushConstantMap[pushConstant.PushConstantName].PushConstantVariableListCount; x++)
                    {
                        GlobalPushContantShaderPushConstantMap[pushConstant.PushConstantName].PushConstantVariableList[x].Value = memorySystem.AddPtrBuffer<byte>(GlobalPushContantShaderPushConstantMap[pushConstant.PushConstantName].PushConstantVariableList[x].Size, __FILE__, __LINE__, __func__);
                    }
                }
                else
                {
                    ShaderPushConstantSourceMap[pushConstant.PushConstantName] = pushConstant;
                    GlobalPushContantShaderPushConstantMap[pushConstant.PushConstantName] = pushConstant;
                    GlobalPushContantShaderPushConstantMap[pushConstant.PushConstantName].PushConstantBuffer = memorySystem.AddPtrBuffer<byte>(pushConstant.PushConstantSize, __FILE__, __LINE__, __func__);

                    for (int x = 0; x < GlobalPushContantShaderPushConstantMap[pushConstant.PushConstantName].PushConstantVariableListCount; x++)
                    {
                        GlobalPushContantShaderPushConstantMap[pushConstant.PushConstantName].PushConstantVariableList[x].Value = memorySystem.AddPtrBuffer<byte>(GlobalPushContantShaderPushConstantMap[pushConstant.PushConstantName].PushConstantVariableList[x].Size, __FILE__, __LINE__, __func__);
                    }
                }
            }
        }
    }
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

void ShaderSystem::UpdateGlobalShaderBuffer(const String& pushConstantName)
{
    if (!ShaderPushConstantExists(pushConstantName))
    {
        std::cerr << "Error: Push constant '" << pushConstantName << "' does not exist!" << std::endl;
        return;
    }

    const ShaderPushConstant& pushConstant = GlobalPushContantShaderPushConstantMap[pushConstantName];
    if (pushConstant.PushConstantBuffer == nullptr)
    {
        std::cerr << "Error: PushConstantBuffer is null for push constant '" << pushConstantName << "'!" << std::endl;
        return;
    }

    size_t offset = 0;
    Vector<ShaderVariable> pushConstantVarList(pushConstant.PushConstantVariableList, pushConstant.PushConstantVariableList + pushConstant.PushConstantVariableListCount);
    for (const auto& pushConstantVar : pushConstantVarList)
    {
        size_t paddingSize = pushConstantVar.Size > pushConstantVar.ByteAlignment ? pushConstantVar.Size : pushConstantVar.ByteAlignment;
        void* dest = static_cast<byte*>(pushConstant.PushConstantBuffer) + offset;
        memset(dest, 0, paddingSize);
        memcpy(dest, pushConstantVar.Value, pushConstantVar.Size);
        offset += paddingSize;
    }
}

ShaderPushConstant* ShaderSystem::GetGlobalShaderPushConstant(const String& pushConstantName)
{
    if (ShaderPushConstantExists(pushConstantName))
    {
        return &GlobalPushContantShaderPushConstantMap[pushConstantName];
    }
    return nullptr;
}

SpvReflectShaderModule& ShaderSystem::FindShaderModule(const String& shaderFile)
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
    auto it = ShaderPushConstantSourceMap.find(shaderFile);
    if (it != ShaderPushConstantSourceMap.end())
    {
        return it->second;
    }
    throw std::out_of_range("ShaderPushConstantSourceMap not found for given shader push constant");
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

bool ShaderSystem::ShaderPushConstantExists(const String& pushConstantName)
{
    auto it = ShaderPushConstantSourceMap.find(pushConstantName);
    if (it != ShaderPushConstantSourceMap.end())
    {
        return true;
    }
    return false;
}

void ShaderSystem::GetPushConstantData(const ShaderPushConstant& pushConstant)
{
    std::vector<ShaderVariable> shaderVarList(pushConstant.PushConstantVariableList,
        pushConstant.PushConstantVariableList + pushConstant.PushConstantVariableListCount);
    for (const auto& shaderVar : shaderVarList)
    {
        std::cout << shaderVar.Value << ": Size: " << shaderVar.Size << " Type: " << shaderVar.MemberTypeEnum;
        std::cout << " Value at " << shaderVar.Value << ": ";

        switch (shaderVar.MemberTypeEnum)
        {
        case shaderInt:
        {
            int val = *static_cast<int*>(shaderVar.Value);
            std::cout << val;
            break;
        }
        case shaderUint:
        {
            unsigned int val = *static_cast<unsigned int*>(shaderVar.Value);
            std::cout << val;
            break;
        }
        case shaderFloat:
        {
            float val = *static_cast<float*>(shaderVar.Value);
            std::cout << val;
            break;
        }
        case shaderIvec2:
        {
            int* vec = static_cast<int*>(shaderVar.Value);
            std::cout << "Ivec2(" << vec[0] << ", " << vec[1] << ")";
            break;
        }
        case shaderIvec3:
        {
            int* vec = static_cast<int*>(shaderVar.Value);
            std::cout << "Ivec3(" << vec[0] << ", " << vec[1] << ", " << vec[2] << ")";
            break;
        }
        case shaderIvec4:
        {
            int* vec = static_cast<int*>(shaderVar.Value);
            std::cout << "Ivec4(" << vec[0] << ", " << vec[1] << ", " << vec[2] << ", " << vec[3] << ")";
            break;
        }
        case shaderVec2:
        {
            float* vec = static_cast<float*>(shaderVar.Value);
            std::cout << "Vec2(" << vec[0] << ", " << vec[1] << ")";
            break;
        }
        case shaderVec3:
        {
            float* vec = static_cast<float*>(shaderVar.Value);
            std::cout << "Vec3(" << vec[0] << ", " << vec[1] << ", " << vec[2] << ")";
            break;
        }
        case shaderVec4:
        {
            float* vec = static_cast<float*>(shaderVar.Value);
            std::cout << "Vec4(" << vec[0] << ", " << vec[1] << ", " << vec[2] << ", " << vec[3] << ")";
            break;
        }
        case shaderMat2:
        {
            float* mat = static_cast<float*>(shaderVar.Value);
            std::cout << "Mat2:\n";
            for (int i = 0; i < 4; ++i)
            {
                std::cout << mat[i] << (i % 2 == 1 ? "\n" : " ");
            }
            break;
        }
        case shaderMat3:
        {
            float* mat = static_cast<float*>(shaderVar.Value);
            std::cout << "Mat3:\n";
            for (int i = 0; i < 9; ++i)
            {
                std::cout << mat[i] << (i % 3 == 2 ? "\n" : " ");
            }
            break;
        }
        case shaderMat4:
        {
            float* mat = static_cast<float*>(shaderVar.Value);
            std::cout << "Mat4:\n";
            for (int i = 0; i < 16; ++i)
            {
                std::cout << mat[i] << (i % 4 == 3 ? "\n" : " ");
            }
            break;
        }
        case shaderbool:
        {
            bool val = *static_cast<bool*>(shaderVar.Value);
            std::cout << (val ? "true" : "false");
            break;
        }
        default:
            std::cout << "Unknown type";
        }

        std::cout << "\n";
    }
    std::cout << std::endl << std::endl;
}