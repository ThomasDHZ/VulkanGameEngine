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

ShaderModule ShaderSystem::AddShaderModule(const String& modulePath, const VkGuid& renderPassId, const VkGuid& pipelineId, const VkGuid& levelId)
{
    Vector<VkDescriptorBufferInfo> vertexPropertiesList = shaderSystem.GetVertexPropertiesBuffer();
    Vector<VkDescriptorBufferInfo> indexPropertiesList = shaderSystem.GetIndexPropertiesBuffer();
    Vector<VkDescriptorBufferInfo> transformPropertiesList = shaderSystem.GetGameObjectTransformBuffer();
    Vector<VkDescriptorBufferInfo> meshPropertiesList = shaderSystem.GetMeshPropertiesBuffer(levelId);
    Vector<VkDescriptorImageInfo> texturePropertiesList = shaderSystem.GetTexturePropertiesBuffer(renderPassId);
    Vector<VkDescriptorBufferInfo> materialPropertiesList = materialSystem.GetMaterialPropertiesBuffer();
    GPUIncludes gpuIncludes =
    {
        .VertexPropertiesCount = vertexPropertiesList.size(),
        .IndexPropertiesCount = indexPropertiesList.size(),
        .TransformPropertiesCount = transformPropertiesList.size(),
        .MeshPropertiesCount = meshPropertiesList.size(),
        .TexturePropertiesListCount = texturePropertiesList.size(),
        .MaterialPropertiesCount = materialPropertiesList.size(),
        .VertexProperties = vertexPropertiesList.data(),
        .IndexProperties = indexPropertiesList.data(),
        .TransformProperties = transformPropertiesList.data(),
        .MeshProperties = meshPropertiesList.data(),
        .TexturePropertiesList = texturePropertiesList.data(),
        .MaterialProperties = materialPropertiesList.data()
    };

    const char* fileName = File_GetFileNameFromPath(modulePath.c_str());
    if (!ShaderModuleExists(fileName))
    {
        ShaderModuleMap[fileName] = Shader_GetShaderData(modulePath, gpuIncludes);
        Span<ShaderPushConstant> pushConstantList(ShaderModuleMap[fileName].PushConstantList, ShaderModuleMap[fileName].PushConstantCount);
        for (auto& pushConstant : pushConstantList)
        {
            pushConstant.PushConstantBuffer = memorySystem.AddPtrBuffer<byte>(pushConstant.PushConstantSize, __FILE__, __LINE__, __func__, pushConstant.PushConstantName.c_str());
            for (int x = 0; x < pushConstant.PushConstantVariableListCount; x++)
            {
                pushConstant.PushConstantVariableList[x].Value = memorySystem.AddPtrBuffer<byte>(pushConstant.PushConstantVariableList[x].Size, __FILE__, __LINE__, __func__, pushConstant.PushConstantVariableList[x].Name.c_str());
            }
            ShaderPushConstantMap[pushConstant.PushConstantName] = pushConstant;
        }

        UnorderedMap<String, ShaderStruct> shaderStructMap;
        Span<ShaderStruct> structList(ShaderModuleMap[fileName].ShaderStructList, ShaderModuleMap[fileName].ShaderStructCount);
        for (auto& structVar : structList)
        {
            structVar.ShaderStructBuffer = memorySystem.AddPtrBuffer<byte>(structVar.ShaderBufferVariableListCount, __FILE__, __LINE__, __func__, structVar.Name.c_str());
            for (int x = 0; x < structVar.ShaderBufferVariableListCount; x++)
            {
                structVar.ShaderBufferVariableList[x].Value = memorySystem.AddPtrBuffer<byte>(structVar.ShaderBufferVariableListCount, __FILE__, __LINE__, __func__, structVar.Name.c_str());
            }
            shaderStructMap[structVar.Name] = structVar;
        }
        PipelineShaderStructMap[pipelineId] = shaderStructMap;
        return ShaderModuleMap[fileName];
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

ShaderPushConstant* ShaderSystem::GetGlobalShaderPushConstant(const String& pushConstantName)
{
    if (ShaderPushConstantExists(pushConstantName))
    {
        return &ShaderPushConstantMap[pushConstantName];
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
    auto it = ShaderPushConstantMap.find(shaderFile);
    if (it != ShaderPushConstantMap.end())
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
    auto it = ShaderPushConstantMap.find(pushConstantName);
    if (it != ShaderPushConstantMap.end())
    {
        return true;
    }
    return false;
}

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

const Vector<VkDescriptorBufferInfo> ShaderSystem::GetMeshPropertiesBuffer(const VkGuid& levelLayerId)
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


const Vector<VkDescriptorImageInfo> ShaderSystem::GetTexturePropertiesBuffer(const VkGuid& renderPassId)
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

void ShaderSystem::Destroy()
{
    for (auto& pushConstant : ShaderPushConstantMap)
    {
        Shader_DestroyConstantBufferVariableData(&pushConstant.second, pushConstant.second.PushConstantVariableListCount);
    }
    for (auto& pushConstant : ShaderPushConstantMap)
    {
        memorySystem.RemovePtrBuffer<ShaderPushConstant>(&pushConstant.second);
    }
    for (auto& shaderModule : ShaderModuleMap)
    {
        Shader_ShaderDestroy(shaderModule.second);
    }
}
