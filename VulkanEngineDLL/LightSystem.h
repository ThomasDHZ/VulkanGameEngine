#pragma once
#include "Platform.h"
#include "JsonStruct.h"

enum LightTypeEnum
{
    kDirectionalLight,
    kPointLight
};

struct DirectionalLightHeader
{
    uint LightOffset;
    uint LightCount;
    uint LightSize;
};

struct DirectionalLight
{
    vec3   LightColor = vec3(1.0f, 1.0f, 1.0f);
    vec3   LightDirection = vec3(0.3f, 0.3f, 1.0f);
    float  LightIntensity = 1.5f;
    float ShadowStrength = 1.0f;
    float ShadowBias = 0.012f;
    float ShadowSoftness = 0.008f;
};

struct PointLightHeader
{
    uint LightOffset;
    uint LightCount;
    uint LightSize;
};

struct PointLight
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
    uint32 PointLightBufferId = UINT32_MAX;
    uint32 DirectionalLightBufferId = UINT32_MAX;
    Vector<DirectionalLight> DirectionalLightList;
    Vector<PointLight>       PointLightList;

    DLL_EXPORT void StartUp();
    DLL_EXPORT void Update(const float& deltaTime, Vector<VulkanPipeline>& pipelineList);
    DLL_EXPORT void LoadSceneLights(const String& directionalLightPath);
    DLL_EXPORT const Vector<VkDescriptorBufferInfo> GetDirectionalLightPropertiesBuffer() const;
    DLL_EXPORT const Vector<VkDescriptorBufferInfo> GetPointLightPropertiesBuffer() const;
};
extern DLL_EXPORT LightSystem& lightSystem;
inline LightSystem& LightSystem::Get()
{
    static LightSystem instance;
    return instance;
}

