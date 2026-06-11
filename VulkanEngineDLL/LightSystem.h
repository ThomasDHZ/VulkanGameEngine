#pragma once
#include "Platform.h"
#include "JsonStruct.h"
#include "MemoryPoolSystem.h"
#include "GameObjectSystem.h"
#include "ComponentSystem.h"

struct DirectionalLightComponent
{
    vec3   LightColor = vec3(1.0f, 1.0f, 1.0f);
    vec3   LightDirection = vec3(0.3f, 0.3f, 1.0f);
    float  LightIntensity = 1.5f;
    float  ShadowStrength = 1.0f;
    float  ShadowBias = 0.012f;
    float  ShadowSoftness = 0.008f;
};

struct PointLightComponent
{
    vec3   LightPosition = vec3(0.0f);
    vec3   LightColor = vec3(1.0f, 0.95f, 0.8f);
    float  LightRadius = 200.0f;
    float  LightIntensity = 2.0f;
    float  ShadowStrength = 1.0f;
    float  ShadowBias = 0.012f;
    float  ShadowSoftness = 0.008f;
};

class LightSystem
{
public:
    static LightSystem& Get();

private:
    LightSystem() = default;
    ~LightSystem() = default;
    LightSystem(const LightSystem&) = delete;
    LightSystem& operator=(const LightSystem&) = delete;
    LightSystem(LightSystem&&) = delete;
    LightSystem& operator=(LightSystem&&) = delete;

public:
    DLL_EXPORT uint32                            LoadLight(const nlohmann::json& json);
    DLL_EXPORT uint32                            AllocateLight(GameObjectTypeEnum lightType);
    DLL_EXPORT DirectionalLightComponent&        GetDirectionalLight(uint directionalLightId);
    DLL_EXPORT PointLightComponent&              GetPointLight(uint pointLightId);
};
extern DLL_EXPORT LightSystem& lightSystem;
inline LightSystem& LightSystem::Get()
{
    static LightSystem instance;
    return instance;
}

