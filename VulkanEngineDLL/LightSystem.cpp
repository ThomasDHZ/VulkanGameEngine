#include "LightSystem.h"
#include "FileSystem.h"
#include "BufferSystem.h"
#include "ShaderSystem.h"
#include "RenderSystem.h"
#include "GameObjectSystem.h"

LightSystem& lightSystem = LightSystem::Get();

uint32 LightSystem::LoadLight(const nlohmann::json& json)
{
    uint lightType = json["ComponentType"];
    switch (lightType)
    {
        case kDirectionalLightComponent:
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
            return poolIndex;
        }
        case kPointLightComponent:
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
            return poolIndex;
        }
    }

    return UINT32_MAX;
}

uint LightSystem::AllocateLight(GameObjectTypeEnum lightType)
{
    switch (lightType)
    {
        case kDirectionalLightComponent: return memoryPoolSystem.AllocateObject(kDirectionalLightBuffer); break;
        case kPointLightComponent:       return memoryPoolSystem.AllocateObject(kPointLightBuffer);  break;
        default: std::cerr << "Not a Light Component" << std::endl;
    }
    return UINT32_MAX;
}

DirectionalLight& LightSystem::GetDirectionalLight(uint directionalLightId)
{
    return memoryPoolSystem.UpdateDirectionalLight(directionalLightId);
}


PointLight& LightSystem::GetPointLight(uint pointLightId)
{
    return memoryPoolSystem.UpdatePointLight(pointLightId);
}

uint LightSystem::FindDirectionalLightIndex(void* ptr)
{
    return memoryPoolSystem.FindDirectionalLightIndex(ptr);
}

uint LightSystem::FindPointLightIndex(void* ptr)
{
    return memoryPoolSystem.FindPointLightIndex(ptr);
}

