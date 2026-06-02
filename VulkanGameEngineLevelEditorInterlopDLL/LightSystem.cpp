#include "LightSystem.h"

uint32 LightSystem_LoadLight(const char* sceneLight)
{
    return lightSystem.LoadLight(sceneLight);
}

uint32 LightSystem_AllocateLight(GameObjectTypeEnum lightType)
{
    return lightSystem.AllocateLight(lightType);
}

DirectionalLightComponent& LightSystem_GetDirectionalLight(uint directionalLightId)
{
    return lightSystem.GetDirectionalLight(directionalLightId);
}

PointLightComponent& LightSystem_GetPointLight(uint pointLightId)
{
    return lightSystem.GetPointLight(pointLightId);
}
