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

ShaderModule ShaderSystem::AddShaderModule(const String& modulePath, VkGuid levelId, VkGuid renderPassId)
{
    Vector<VkDescriptorBufferInfo> vertexPropertiesList = shaderSystem.GetVertexPropertiesBuffer();
    Vector<VkDescriptorBufferInfo> indexPropertiesList = shaderSystem.GetIndexPropertiesBuffer();
    Vector<VkDescriptorBufferInfo> transformPropertiesList = shaderSystem.GetGameObjectTransformBuffer();
    Vector<VkDescriptorBufferInfo> meshPropertiesList = shaderSystem.GetMeshPropertiesBuffer(levelId);
    Vector<VkDescriptorImageInfo> texturePropertiesList = shaderSystem.GetTexturePropertiesBuffer(renderPassId);
    Vector<VkDescriptorBufferInfo> materialPropertiesList = materialSystem.GetMaterialPropertiesBuffer();

    const char* fileName = File_GetFileNameFromPath(modulePath.c_str());
    if (!ShaderModuleExists(fileName))
    {
        ShaderModuleMap[fileName] = Shader_GetShaderData(modulePath);
        Vector<ShaderPushConstant> pushConstantList = Vector<ShaderPushConstant>(ShaderModuleMap[fileName].PushConstantList, ShaderModuleMap[fileName].PushConstantList + ShaderModuleMap[fileName].PushConstantCount);
        for (auto& pushConstant : pushConstantList)
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

            if (ShaderModuleMap[fileName].DescriptorBindingCount)
            {
                Vector<ShaderDescriptorBinding> bindingList;
                for (int x = 0; x < ShaderModuleMap[fileName].DescriptorBindingCount; x++)
                {
                    switch (ShaderModuleMap[fileName].DescriptorBindingsList[x].DescriptorBindingType)
                    {
                        case kVertexDescsriptor:
                        {
                            ShaderModuleMap[fileName].DescriptorBindingsList[x].DescriptorBufferInfo = memorySystem.AddPtrBuffer<VkDescriptorBufferInfo>(vertexPropertiesList.size(), __FILE__, __LINE__, __func__);
                            std::memcpy(ShaderModuleMap[fileName].DescriptorBindingsList[x].DescriptorBufferInfo, vertexPropertiesList.data(), vertexPropertiesList.size() * sizeof(VkDescriptorBufferInfo));
                            break;
                        }
                        case kIndexDescriptor:
                        {
                            ShaderModuleMap[fileName].DescriptorBindingsList[x].DescriptorBufferInfo = memorySystem.AddPtrBuffer<VkDescriptorBufferInfo>(indexPropertiesList.size(), __FILE__, __LINE__, __func__);
                            std::memcpy(ShaderModuleMap[fileName].DescriptorBindingsList[x].DescriptorBufferInfo, indexPropertiesList.data(), indexPropertiesList.size() * sizeof(VkDescriptorBufferInfo));
                            break;
                        }
                        case kTransformDescriptor:
                        {
                            ShaderModuleMap[fileName].DescriptorBindingsList[x].DescriptorBufferInfo = memorySystem.AddPtrBuffer<VkDescriptorBufferInfo>(transformPropertiesList.size(), __FILE__, __LINE__, __func__);
                            std::memcpy(ShaderModuleMap[fileName].DescriptorBindingsList[x].DescriptorBufferInfo, transformPropertiesList.data(), transformPropertiesList.size() * sizeof(VkDescriptorBufferInfo));
                            break;
                        }
                        case kMeshPropertiesDescriptor:
                        {
                            ShaderModuleMap[fileName].DescriptorBindingsList[x].DescriptorBufferInfo = memorySystem.AddPtrBuffer<VkDescriptorBufferInfo>(meshPropertiesList.size(), __FILE__, __LINE__, __func__);
                            std::memcpy(ShaderModuleMap[fileName].DescriptorBindingsList[x].DescriptorBufferInfo, meshPropertiesList.data(), meshPropertiesList.size() * sizeof(VkDescriptorBufferInfo));
                            break;
                        }
                        case kTextureDescriptor:
                        {
                            ShaderModuleMap[fileName].DescriptorBindingsList[x].DescriptorImageInfo = memorySystem.AddPtrBuffer<VkDescriptorImageInfo>(texturePropertiesList.size(), __FILE__, __LINE__, __func__);
                            std::memcpy(ShaderModuleMap[fileName].DescriptorBindingsList[x].DescriptorImageInfo, texturePropertiesList.data(), texturePropertiesList.size() * sizeof(VkDescriptorImageInfo));
                            break;
                        }
                        case kMaterialDescriptor:
                        {
                            ShaderModuleMap[fileName].DescriptorBindingsList[x].DescriptorBufferInfo = memorySystem.AddPtrBuffer<VkDescriptorBufferInfo>(materialPropertiesList.size(), __FILE__, __LINE__, __func__);
                            std::memcpy(ShaderModuleMap[fileName].DescriptorBindingsList[x].DescriptorBufferInfo, materialPropertiesList.data(), materialPropertiesList.size() * sizeof(VkDescriptorBufferInfo));
                            break;
                        }
                        default:
                        {
                            throw std::runtime_error("Binding case hasn't been handled yet");
                        }
                    }
                }
            }
            return ShaderModuleMap[fileName];
        }
    }
    return ShaderModuleMap[fileName];
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
    Vector<ShaderVariable> pushConstantVarList(pushConstant.PushConstantVariableList,
        pushConstant.PushConstantVariableList + pushConstant.PushConstantVariableListCount);
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

ShaderPushConstant* ShaderSystem::GetGlobalShaderPushConstant(const String& pushConstantName)
{
    if (ShaderPushConstantExists(pushConstantName))
    {
        return &GlobalPushContantShaderPushConstantMap[pushConstantName];
    }
    return nullptr;
}

ShaderModule& ShaderSystem::FindShaderModule(const String& shaderFile)
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

//void ShaderSystem::GetPushConstantData(const ShaderPushConstant& pushConstant)
//{
//    std::vector<ShaderVariable> shaderVarList(pushConstant.PushConstantVariableList,
//        pushConstant.PushConstantVariableList + pushConstant.PushConstantVariableListCount);
//    for (const auto& shaderVar : shaderVarList)
//    {
//        std::cout << shaderVar.Value << ": Size: " << shaderVar.Size << " Type: " << shaderVar.MemberTypeEnum;
//        std::cout << " Value at " << shaderVar.Value << ": ";
//
//        switch (shaderVar.MemberTypeEnum)
//        {
//        case shaderInt:
//        {
//            int val = *static_cast<int*>(shaderVar.Value);
//            std::cout << val;
//            break;
//        }
//        case shaderUint:
//        {
//            unsigned int val = *static_cast<unsigned int*>(shaderVar.Value);
//            std::cout << val;
//            break;
//        }
//        case shaderFloat:
//        {
//            float val = *static_cast<float*>(shaderVar.Value);
//            std::cout << val;
//            break;
//        }
//        case shaderIvec2:
//        {
//            int* vec = static_cast<int*>(shaderVar.Value);
//            std::cout << "Ivec2(" << vec[0] << ", " << vec[1] << ")";
//            break;
//        }
//        case shaderIvec3:
//        {
//            int* vec = static_cast<int*>(shaderVar.Value);
//            std::cout << "Ivec3(" << vec[0] << ", " << vec[1] << ", " << vec[2] << ")";
//            break;
//        }
//        case shaderIvec4:
//        {
//            int* vec = static_cast<int*>(shaderVar.Value);
//            std::cout << "Ivec4(" << vec[0] << ", " << vec[1] << ", " << vec[2] << ", " << vec[3] << ")";
//            break;
//        }
//        case shaderVec2:
//        {
//            float* vec = static_cast<float*>(shaderVar.Value);
//            std::cout << "Vec2(" << vec[0] << ", " << vec[1] << ")";
//            break;
//        }
//        case shaderVec3:
//        {
//            float* vec = static_cast<float*>(shaderVar.Value);
//            std::cout << "Vec3(" << vec[0] << ", " << vec[1] << ", " << vec[2] << ")";
//            break;
//        }
//        case shaderVec4:
//        {
//            float* vec = static_cast<float*>(shaderVar.Value);
//            std::cout << "Vec4(" << vec[0] << ", " << vec[1] << ", " << vec[2] << ", " << vec[3] << ")";
//            break;
//        }
//        case shaderMat2:
//        {
//            float* mat = static_cast<float*>(shaderVar.Value);
//            std::cout << "Mat2:\n";
//            for (int i = 0; i < 4; ++i)
//            {
//                std::cout << mat[i] << (i % 2 == 1 ? "\n" : " ");
//            }
//            break;
//        }
//        case shaderMat3:
//        {
//            float* mat = static_cast<float*>(shaderVar.Value);
//            std::cout << "Mat3:\n";
//            for (int i = 0; i < 9; ++i)
//            {
//                std::cout << mat[i] << (i % 3 == 2 ? "\n" : " ");
//            }
//            break;
//        }
//        case shaderMat4:
//        {
//            float* mat = static_cast<float*>(shaderVar.Value);
//            std::cout << "Mat4:\n";
//            for (int i = 0; i < 16; ++i)
//            {
//                std::cout << mat[i] << (i % 4 == 3 ? "\n" : " ");
//            }
//            break;
//        }
//        case shaderbool:
//        {
//            bool val = *static_cast<bool*>(shaderVar.Value);
//            std::cout << (val ? "true" : "false");
//            break;
//        }
//        default:
//            std::cout << "Unknown type";
//        }
//
//        std::cout << "\n";
//    }
//    std::cout << std::endl << std::endl;
//}

const Vector<VkDescriptorBufferInfo> ShaderSystem::GetVertexPropertiesBuffer()
{
    //Vector<MeshStruct> meshList;
    //meshList.reserve(meshSystem.SpriteMeshList.size());
    //std::transform(meshSystem.SpriteMeshList.begin(), meshSystem.SpriteMeshList.end(),
    //    std::back_inserter(meshList),
    //    [](const auto& pair) { return pair.second; });


    Vector<VkDescriptorBufferInfo> vertexPropertiesBuffer;
    //if (meshList.empty())
    //{
    //    vertexPropertiesBuffer.emplace_back(VkDescriptorBufferInfo
    //        {
    //            .buffer = VK_NULL_HANDLE,
    //            .offset = 0,
    //            .range = VK_WHOLE_SIZE
    //        });
    //}
    //else
    //{
    //    for (auto& mesh : meshList)
    //    {
    //        const VulkanBufferStruct& vertexProperties = bufferSystem.VulkanBuffer[mesh.MeshVertexBufferId];
    //        vertexPropertiesBuffer.emplace_back(VkDescriptorBufferInfo
    //            {
    //                .buffer = vertexProperties.Buffer,
    //                .offset = 0,
    //                .range = VK_WHOLE_SIZE
    //            });
    //    }
    //}

    return vertexPropertiesBuffer;
}

const Vector<VkDescriptorBufferInfo> ShaderSystem::GetIndexPropertiesBuffer()
{
    //Vector<MeshStruct> meshList;
    //meshList.reserve(meshSystem.SpriteMeshList.size());
    //std::transform(meshSystem.SpriteMeshList.begin(), meshSystem.SpriteMeshList.end(),
    //    std::back_inserter(meshList),
    //    [](const auto& pair) { return pair.second; });

    std::vector<VkDescriptorBufferInfo>	indexPropertiesBuffer;
    //if (meshList.empty())
    //{
    //    indexPropertiesBuffer.emplace_back(VkDescriptorBufferInfo
    //        {
    //            .buffer = VK_NULL_HANDLE,
    //            .offset = 0,
    //            .range = VK_WHOLE_SIZE
    //        });
    //}
    //else
    //{
    //    for (auto& mesh : meshList)
    //    {
    //        const VulkanBufferStruct& indexProperties = bufferSystem.VulkanBuffer[mesh.MeshIndexBufferId];
    //        indexPropertiesBuffer.emplace_back(VkDescriptorBufferInfo
    //            {
    //                .buffer = indexProperties.Buffer,
    //                .offset = 0,
    //                .range = VK_WHOLE_SIZE
    //            });
    //    }
    //}
    return indexPropertiesBuffer;
}

const Vector<VkDescriptorBufferInfo> ShaderSystem::GetGameObjectTransformBuffer()
{
    //Vector<MeshStruct> meshList;
    //meshList.reserve(meshSystem.SpriteMeshList.size());
    //std::transform(meshSystem.SpriteMeshList.begin(), meshSystem.SpriteMeshList.end(),
    //    std::back_inserter(meshList),
    //    [](const auto& pair) { return pair.second; });

    std::vector<VkDescriptorBufferInfo>	transformPropertiesBuffer;
    //if (meshList.empty())
    //{
    //    transformPropertiesBuffer.emplace_back(VkDescriptorBufferInfo
    //        {
    //            .buffer = VK_NULL_HANDLE,
    //            .offset = 0,
    //            .range = VK_WHOLE_SIZE
    //        });
    //}
    //else
    //{
    //    for (auto& mesh : meshList)
    //    {
    //        const VulkanBufferStruct& transformBuffer = bufferSystem.VulkanBuffer[mesh.MeshTransformBufferId];
    //        transformPropertiesBuffer.emplace_back(VkDescriptorBufferInfo
    //            {
    //                .buffer = transformBuffer.Buffer,
    //                .offset = 0,
    //                .range = VK_WHOLE_SIZE
    //            });
    //    }
    //}

    return transformPropertiesBuffer;
}

const Vector<VkDescriptorBufferInfo> ShaderSystem::GetMeshPropertiesBuffer(VkGuid& levelLayerId)
{
    Vector<Mesh> meshList;
    if (levelLayerId == VkGuid())
    {
        for (auto& sprite : meshSystem.SpriteMeshList())
        {
            meshList.emplace_back(sprite);

        }
    }
    else
    {
        for (auto& layer : meshSystem.FindLevelLayerMeshList(levelLayerId))
        {
            meshList.emplace_back(layer);
        }
    }

    Vector<VkDescriptorBufferInfo> meshPropertiesBuffer;
    if (meshList.empty())
    {
        meshPropertiesBuffer.emplace_back(VkDescriptorBufferInfo
            {
                .buffer = VK_NULL_HANDLE,
                .offset = 0,
                .range = VK_WHOLE_SIZE
            });
    }
    else
    {
        for (auto& mesh : meshList)
        {
            const VulkanBuffer& meshProperties = bufferSystem.FindVulkanBuffer(mesh.PropertiesBufferId);
            meshPropertiesBuffer.emplace_back(VkDescriptorBufferInfo
                {
                    .buffer = meshProperties.Buffer,
                    .offset = 0,
                    .range = VK_WHOLE_SIZE
                });
        }
    }

    return meshPropertiesBuffer;
}


const Vector<VkDescriptorImageInfo> ShaderSystem::GetTexturePropertiesBuffer(VkGuid& renderPassId)
{
    Vector<Texture> textureList;
    const VulkanRenderPass& renderPass = renderSystem.FindRenderPass(renderPassId);
    if (renderPass.InputTextureIdListCount > 0)
    {
        Vector<VkGuid> inputTextureList = Vector<VkGuid>(renderPass.InputTextureIdList, renderPass.InputTextureIdList + renderPass.InputTextureIdListCount);
        for (auto& inputTexture : inputTextureList)
        {
            textureList.emplace_back(textureSystem.FindRenderedTexture(inputTexture));
        }
    }
    else
    {
        textureList = textureSystem.TextureList();
    }

    Vector<VkDescriptorImageInfo>	texturePropertiesBuffer;
    if (textureList.empty())
    {
        VkSamplerCreateInfo NullSamplerInfo =
        {
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .magFilter = VK_FILTER_NEAREST,
            .minFilter = VK_FILTER_NEAREST,
            .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
            .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .mipLodBias = 0,
            .anisotropyEnable = VK_TRUE,
            .maxAnisotropy = 16.0f,
            .compareEnable = VK_FALSE,
            .compareOp = VK_COMPARE_OP_ALWAYS,
            .minLod = 0,
            .maxLod = 0,
            .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
            .unnormalizedCoordinates = VK_FALSE,
        };

        VkSampler nullSampler = VK_NULL_HANDLE;
        if (vkCreateSampler(renderSystem.renderer.Device, &NullSamplerInfo, nullptr, &nullSampler))
        {
            throw std::runtime_error("Failed to create Sampler.");
        }

        VkDescriptorImageInfo nullBuffer =
        {
            .sampler = nullSampler,
            .imageView = VK_NULL_HANDLE,
            .imageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        };
        texturePropertiesBuffer.emplace_back(nullBuffer);
    }
    else
    {
        for (auto& texture : textureList)
        {
            textureSystem.GetTexturePropertiesBuffer(texture, texturePropertiesBuffer);
        }
    }

    return texturePropertiesBuffer;
}