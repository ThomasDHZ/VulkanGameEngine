#include "LightSystem.h"

uint32 LightSystem_LoadLight(const char* sceneLight)
{
    return lightSystem.LoadLight(sceneLight);
}

uint32 LightSystem_AllocateLight(LightTypeEnum lightType)
{
    return lightSystem.AllocateLight(lightType);
}

DirectionalLight& LightSystem_GetDirectionalLight(uint directionalLightId)
{
    return lightSystem.GetDirectionalLight(directionalLightId);
}

PointLight& LightSystem_GetPointLight(uint pointLightId)
{
    return lightSystem.GetPointLight(pointLightId);
}
