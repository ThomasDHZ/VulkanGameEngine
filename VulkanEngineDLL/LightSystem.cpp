#include "LightSystem.h"
#include "FileSystem.h"
#include "BufferSystem.h"
#include "ShaderSystem.h"
#include "RenderSystem.h"

LightSystem& lightSystem = LightSystem::Get();

void LightSystem::StartUp()
{
    //constexpr size_t InitialCapacity = 4096;
    //DirectionalLightPool.CreateMemoryPool(InitialCapacity, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    //PointLightPool.CreateMemoryPool(InitialCapacity, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
}

void LightSystem::Update(const float& deltaTime, Vector<VulkanPipeline>& pipelineList)
{

}

void LightSystem::LoadSceneLights(const String& directionalLightPath)
{
    nlohmann::json json = fileSystem.LoadJsonFile(directionalLightPath.c_str());
    uint lightType = json["LightType"];
    switch (lightType)
    {
        case kDirectionalLight:
        {
            uint32 poolIndex = memoryPoolSystem.AllocateObject(kDirectionalLightBuffer);
            DirectionalLight& directionalLight = memoryPoolSystem.UpdateDirectionalLight(poolIndex);
            directionalLight = DirectionalLight
                {
                    .LightColor = vec3(json["LightColor"][0], json["LightColor"][1], json["LightColor"][2]),
                    .LightDirection = vec3(json["LightDirection"][0], json["LightDirection"][1], json["LightDirection"][2]),
                    .LightIntensity = json["LightIntensity"],
                    .ShadowStrength = json["ShadowStrength"],
                    .ShadowBias = json["ShadowBias"],
                    .ShadowSoftness = json["ShadowSoftness"],
                };
            break;
        }
        case kPointLight:
        {
            uint32 poolIndex = memoryPoolSystem.AllocateObject(kPointLightBuffer);
            PointLight& pointLight = memoryPoolSystem.UpdatePointLight(poolIndex);
            pointLight = PointLight
                {
                    .LightPosition = vec3(json["LightPosition"][0], json["LightPosition"][1], json["LightPosition"][2]),
                    .LightColor = vec3(json["LightColor"][0], json["LightColor"][1], json["LightColor"][2]),
                    .LightRadius = json["LightRadius"],
                    .LightIntensity = json["LightIntensity"],
                    .ShadowStrength = json["ShadowStrength"],
                    .ShadowBias = json["ShadowBias"],
                    .ShadowSoftness = json["ShadowSoftness"],
                };
            break;
        }
    }
}
