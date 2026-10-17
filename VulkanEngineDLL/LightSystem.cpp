#include "LightSystem.h"
#include "FileSystem.h"
#include "BufferSystem.h"
#include "ShaderSystem.h"
#include "RenderSystem.h"
#include "GameObjectSystem.h"
#include "CameraSystem.h"
#include "SpriteSystem.h"

LightSystem& lightSystem = LightSystem::Get();

void LightSystem::UpdateDirectionalLightViewProjection(DirectionalLightComponent& lightComponent)
{
    SpriteInstance playerSprite = spriteSystem.FindPlayerSpriteInstance();

    DirectionalLight& light = memoryPoolSystem.UpdateDirectionalLight(lightComponent.DirectionalLightMemoryPoolIndex);
    vec3 L = glm::normalize(light.LightDirection);
    vec3 up = abs(L.y) > 0.99f ? vec3(0.0f, 0.0f, 1.0f) : vec3(0.0f, 1.0f, 0.0f);

    vec2 half = playerSprite.SpriteSize * 0.5f;
    vec3 sceneCenter = vec3(playerSprite.SpritePosition.x + half.x, playerSprite.SpritePosition.y + half.y, 0.0f);
    vec2 sceneSize   = vec2(4296.0f);
    float dist = glm::length(sceneSize) + LayerDistance;

    vec3 eye = sceneCenter - L * dist;
    mat4 view = glm::lookAt(eye, sceneCenter, up);

    float halfW = sceneSize.x * 0.5f + LayerDistance;
    float halfH = sceneSize.y * 0.5f + LayerDistance;
    mat4 proj = glm::ortho(-halfW, halfW, -halfH, halfH, 0.0f, dist * 2.0f);
    proj[1][1] *= -1.0f;

    light.LightSpaceMatrix = proj * view;
}

void LightSystem::UpdatePointLightViewProjection(PointLightComponent& lightComponent)
{
    PointLight& light = memoryPoolSystem.UpdatePointLight(lightComponent.PointLightMemoryPoolIndex);
    mat4 projectionMatrix = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10000.0f);
    light.LightSpaceMatrix[0] = projectionMatrix * glm::lookAt(light.LightPosition, light.LightPosition + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
    light.LightSpaceMatrix[1] = projectionMatrix * glm::lookAt(light.LightPosition, light.LightPosition + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
    light.LightSpaceMatrix[2] = projectionMatrix * glm::lookAt(light.LightPosition, light.LightPosition + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    light.LightSpaceMatrix[3] = projectionMatrix * glm::lookAt(light.LightPosition, light.LightPosition + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
    light.LightSpaceMatrix[4] = projectionMatrix * glm::lookAt(light.LightPosition, light.LightPosition + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
    light.LightSpaceMatrix[5] = projectionMatrix * glm::lookAt(light.LightPosition, light.LightPosition + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
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
    for (auto [entity, directionalLight] : gameObjectSystem.EntityRegistry.view<DirectionalLightComponent>().each())
    {
        lightSystem.UpdateDirectionalLightViewProjection(directionalLight);
    }
    for (auto [entityite, pointLight] : gameObjectSystem.EntityRegistry.view<PointLightComponent>().each())
    {
        lightSystem.UpdatePointLightViewProjection(pointLight);
    }
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

mat4 LightSystem::GetPointLightProjectionMatrix(PointLightComponent& lightComponent, uint32 viewFaceIndex)
{
    return lightComponent.PointLightFaceViews[viewFaceIndex];
}

uint LightSystem::FindDirectionalLightIndex(void* ptr)
{
    return memoryPoolSystem.FindDirectionalLightIndex(ptr);
}

uint LightSystem::FindPointLightIndex(void* ptr)
{
    return memoryPoolSystem.FindPointLightIndex(ptr);
}

