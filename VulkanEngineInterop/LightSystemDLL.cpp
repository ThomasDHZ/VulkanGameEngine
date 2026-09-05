
#include "LightSystemDLL.h"

DirectionalLightComponent& LightSystem_GetDirectionalLight(uint directionalLightId)
{
    return lightSystem.GetDirectionalLight(directionalLightId);
}

PointLightComponent& LightSystem_GetPointLight(uint pointLightId)
{
    return lightSystem.GetPointLight(pointLightId);
}

uint LightSystem_FindDirectionalLightIndex(void* ptr)
{
    return lightSystem.FindDirectionalLightIndex(ptr);
}

uint LightSystem_FindPointLightIndex(void* ptr)
{
    return lightSystem.FindPointLightIndex(ptr);
}

