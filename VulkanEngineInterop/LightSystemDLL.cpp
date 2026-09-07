
#include "LightSystemDLL.h"

DirectionalLight& LightSystem_GetDirectionalLight(uint directionalLightId)
{
    return lightSystem.GetDirectionalLight(directionalLightId);
}

PointLight& LightSystem_GetPointLight(uint pointLightId)
{
    return lightSystem.GetPointLight(pointLightId);
}
