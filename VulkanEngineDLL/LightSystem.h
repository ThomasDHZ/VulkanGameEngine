#pragma once
#include "Platform.h"

enum LightTypeEnum
{
    kDirectionalLight,
    kPointLight
};

struct DirectionalLight
{
    uint32 DirectionalLightBufferId = UINT32_MAX;
    vec3   LightColor = vec3(1.0f, 1.0f, 1.0f);
    vec3   LightDirection = vec3(0.3f, 0.3f, 1.0f);
    float  LightIntensity = 1.5f;

    bool operator==(const DirectionalLight& rhs) const
    {
        return this->DirectionalLightBufferId == rhs.DirectionalLightBufferId;
    }
};

struct PointLight
{
    uint32 PointLightBufferId = UINT32_MAX;
    vec3   LightPosition = vec3(0.0f);
    vec3   LightColor = vec3(1.0f, 0.95f, 0.8f);
    float  LightRadius = 200.0f;
    float  LightIntensity = 2.0f;

    bool operator==(const PointLight& rhs) const
    {
        return this->PointLightBufferId == rhs.PointLightBufferId;
    }
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
    Vector<DirectionalLight> DirectionalLightList;
    Vector<PointLight>       PointLightList;

    DLL_EXPORT void Update(const float& deltaTime);
    DLL_EXPORT void LoadSceneLights(const String& directionalLightPath);
    DLL_EXPORT void UpdateDirectionalLightOrthographicView(const DirectionalLight& directionalLight);
    DLL_EXPORT void UpdatePointLightOrthographicView(const PointLight& pointLight);
    DLL_EXPORT const Vector<VkDescriptorBufferInfo> GetDirectionalLightPropertiesBuffer();
    DLL_EXPORT const Vector<VkDescriptorBufferInfo> GetPointLightPropertiesBuffer();
};
extern DLL_EXPORT LightSystem& lightSystem;
inline LightSystem& LightSystem::Get()
{
    static LightSystem instance;
    return instance;
}

