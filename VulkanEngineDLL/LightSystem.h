#pragma once
#include "Platform.h"
#include "JsonStruct.h"
#include "MemoryPool.h"
#include "MemoryPoolSystem.h"
enum LightTypeEnum
{
    kDirectionalLight,
    kPointLight
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

    DLL_EXPORT void StartUp();
    DLL_EXPORT void Update(const float& deltaTime, Vector<VulkanPipeline>& pipelineList);
    DLL_EXPORT void LoadSceneLights(const String& directionalLightPath);
};
extern DLL_EXPORT LightSystem& lightSystem;
inline LightSystem& LightSystem::Get()
{
    static LightSystem instance;
    return instance;
}

