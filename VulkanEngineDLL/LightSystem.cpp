#include "LightSystem.h"
#include "FileSystem.h"
#include "BufferSystem.h"
#include "ShaderSystem.h"
#include "RenderSystem.h"

LightSystem& lightSystem = LightSystem::Get();

void LightSystem::StartUp()
{
    DirectionalLightBufferId = bufferSystem.VMACreateDynamicBuffer(nullptr, 4096 * sizeof(DirectionalLight), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    PointLightBufferId = bufferSystem.VMACreateDynamicBuffer(nullptr, 4096 * sizeof(DirectionalLight), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
}

void LightSystem::Update(const float& deltaTime, Vector<VulkanPipeline>& pipelineList)
{
    {
        DirectionalLightHeader dlightHeader =
        {
            .LightOffset = sizeof(DirectionalLightHeader),
            .LightCount = static_cast<uint>(DirectionalLightList.size()),
            .LightSize = sizeof(DirectionalLight),
        };

        Vector<byte> uploadData;
        uploadData.resize(sizeof(DirectionalLightHeader) + DirectionalLightList.size() * sizeof(DirectionalLight));
        memcpy(uploadData.data(), &dlightHeader, sizeof(DirectionalLightHeader));
        memcpy(uploadData.data() + sizeof(DirectionalLightHeader), DirectionalLightList.data(), DirectionalLightList.size() * sizeof(DirectionalLight));

        bool bufferRecreated = false;
        if (DirectionalLightBufferId == UINT32_MAX)
        {
            DirectionalLightBufferId = bufferSystem.VMACreateDynamicBuffer(nullptr, uploadData.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
            bufferRecreated = true;
        }
        else if (bufferSystem.FindVulkanBuffer(DirectionalLightBufferId).BufferSize < uploadData.size())
        {
            bufferSystem.DestroyBuffer(bufferSystem.FindVulkanBuffer(DirectionalLightBufferId));
            DirectionalLightBufferId = bufferSystem.VMACreateDynamicBuffer(nullptr, uploadData.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
            bufferRecreated = true;
        }
        bufferSystem.VMAUpdateDynamicBuffer(DirectionalLightBufferId, uploadData.data(), uploadData.size());

        auto bufferInfo = GetDirectionalLightPropertiesBuffer();
        for (auto& pipeline : pipelineList)
        {
            renderSystem.UpdateDescriptorSet(pipeline, bufferInfo, 11);
        }
    }

    {
        PointLightHeader plightHeader =
        {
            .LightOffset = sizeof(DirectionalLightHeader),
            .LightCount = static_cast<uint>(PointLightList.size()),
            .LightSize = sizeof(PointLight),
        };

        Vector<byte> uploadData;
        uploadData.resize(sizeof(PointLightHeader) + PointLightList.size() * sizeof(PointLight));
        memcpy(uploadData.data(), &plightHeader, sizeof(PointLightHeader));
        memcpy(uploadData.data() + sizeof(PointLightHeader), PointLightList.data(), PointLightList.size() * sizeof(PointLight));

        bool bufferRecreated = false;
        if (PointLightBufferId == UINT32_MAX)
        {
            PointLightBufferId = bufferSystem.VMACreateDynamicBuffer(nullptr, uploadData.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
            bufferRecreated = true;
        }
        else if (bufferSystem.FindVulkanBuffer(PointLightBufferId).BufferSize < uploadData.size())
        {
            bufferSystem.DestroyBuffer(bufferSystem.FindVulkanBuffer(PointLightBufferId));
            PointLightBufferId = bufferSystem.VMACreateDynamicBuffer(nullptr, uploadData.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
            bufferRecreated = true;
        }
        bufferSystem.VMAUpdateDynamicBuffer(PointLightBufferId, uploadData.data(), uploadData.size());

        auto bufferInfo = GetDirectionalLightPropertiesBuffer();
        for (auto& pipeline : pipelineList)
        {
            renderSystem.UpdateDescriptorSet(pipeline, bufferInfo, 12);
        }
    }
}

void LightSystem::LoadSceneLights(const String& directionalLightPath)
{
    nlohmann::json json = fileSystem.LoadJsonFile(directionalLightPath.c_str());
    uint lightType = json["LightType"];
    switch (lightType)
    {
        case kDirectionalLight:
        {
            DirectionalLightList.emplace_back(DirectionalLight
                {
                    .LightColor = vec3(json["LightColor"][0], json["LightColor"][1], json["LightColor"][2]),
                    .LightDirection = vec3(json["LightDirection"][0], json["LightDirection"][1], json["LightDirection"][2]),
                    .LightIntensity = json["LightIntensity"],
                    .ShadowStrength = json["ShadowStrength"],
                    .ShadowBias = json["ShadowBias"],
                    .ShadowSoftness = json["ShadowSoftness"],
                });
            break;
        }
        case kPointLight:
        {
            PointLightList.emplace_back(PointLight
                {
                    .LightPosition = vec3(json["LightPosition"][0], json["LightPosition"][1], json["LightPosition"][2]),
                    .LightColor = vec3(json["LightColor"][0], json["LightColor"][1], json["LightColor"][2]),
                    .LightRadius = json["LightRadius"],
                    .LightIntensity = json["LightIntensity"]
                });
            break;
        }
    }
}

const Vector<VkDescriptorBufferInfo> LightSystem::GetDirectionalLightPropertiesBuffer() const 
{
    return Vector<VkDescriptorBufferInfo>
    {
        VkDescriptorBufferInfo
        {
            .buffer = bufferSystem.FindVulkanBuffer(DirectionalLightBufferId).Buffer,
            .offset = 0,
            .range = VK_WHOLE_SIZE
        }
    };
}

const Vector<VkDescriptorBufferInfo> LightSystem::GetPointLightPropertiesBuffer() const
{
    return Vector<VkDescriptorBufferInfo>
    {
        VkDescriptorBufferInfo
        {
            .buffer = bufferSystem.FindVulkanBuffer(PointLightBufferId).Buffer,
            .offset = 0,
            .range = VK_WHOLE_SIZE
        }
    };
}
