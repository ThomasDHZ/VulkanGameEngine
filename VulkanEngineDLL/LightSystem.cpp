#include "LightSystem.h"
#include "FileSystem.h"
#include "BufferSystem.h"
#include "ShaderSystem.h"
#include "RenderSystem.h"
#include "GameObjectSystem.h"
#include "CameraSystem.h"

LightSystem& lightSystem = LightSystem::Get();

void LightSystem::UpdateDirectionalLightViewProjection(uint lightIndex)
{
    DirectionalLight& light = memoryPoolSystem.UpdateDirectionalLight(lightIndex);
    vec3 L = glm::normalize(light.LightDirection);
    vec3 up = abs(L.y) > 0.99f ? vec3(0.0f, 0.0f, 1.0f) : vec3(0.0f, 1.0f, 0.0f);

    const Camera& ortho = cameraSystem.CameraList[cameraSystem.ActiveCameraIndex];
    vec2 sceneSize = ortho.ViewScreenSize / ortho.Zoom;
    vec3 sceneCenter = vec3(ortho.Position.x + sceneSize.x * 0.5f, ortho.Position.y + sceneSize.y * 0.5f, 0.0f);
    float dist = glm::length(sceneSize) + LayerDistance;
    vec3 eye = sceneCenter - L * dist;
    mat4 view = glm::lookAt(eye, sceneCenter, up);

    float halfW = sceneSize.x * 0.5f + LayerDistance;
    float halfH = sceneSize.y * 0.5f + LayerDistance;
    mat4 proj = glm::ortho(-halfW, halfW, -halfH, halfH, 0.0f, dist * 2.0f);
    proj[1][1] *= -1.0f;

    light.LightSpaceMatrix = proj * view;
}

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

void LightSystem::Update()
{
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

