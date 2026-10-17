#pragma once
#include "DLL.h"
#include <Platform.h>
#include "JsonStruct.h"
#include "MemoryPoolSystem.h"
#include "GameObjectSystem.h"
#include "ComponentSystem.h"

struct PointLightComponent
{
    uint PointLightMemoryPoolIndex = UINT32_MAX;
    uint PointLightCameraId = UINT32_MAX;
    std::array<mat4, 6> PointLightFaceViews;
};

struct DirectionalLightComponent
{
    uint DirectionalLightMemoryPoolIndex = UINT32_MAX;
    uint DirectionalLightCameraId = UINT32_MAX;
};

class ENGINE_DLL_EXPORT  LightSystem
{
public:
    static LightSystem& Get();
    static constexpr float LayerDistance = 32.0f;
private:
    LightSystem() = default;
    ~LightSystem() = default;
    LightSystem(const LightSystem&) = delete;
    LightSystem& operator=(const LightSystem&) = delete;
    LightSystem(LightSystem&&) = delete;
    LightSystem& operator=(LightSystem&&) = delete;

     void                     UpdateDirectionalLightViewProjection(DirectionalLightComponent& lightComponent);
     void                     UpdatePointLightViewProjection(PointLightComponent& lightComponent);

public:
     uint32                   LoadLight(const nlohmann::json& json);
     void                     Update();
     uint32                   AllocateLight(GameObjectTypeEnum lightType);
     DirectionalLight&        GetDirectionalLight(uint directionalLightId);
     PointLight&              GetPointLight(uint pointLightId);
     mat4                     GetPointLightProjectionMatrix(PointLightComponent& lightComponent, uint32 viewFace);

      uint                    FindDirectionalLightIndex(void* ptr);
      uint                    FindPointLightIndex(void* ptr);
};
ENGINE_DLL_EXPORT extern  LightSystem& lightSystem;
inline LightSystem& LightSystem::Get()
{
    static LightSystem instance;
    return instance;
}

