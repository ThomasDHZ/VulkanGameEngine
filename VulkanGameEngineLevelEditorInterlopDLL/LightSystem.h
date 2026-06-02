#pragma once
#include <LightSystem.h>

#ifdef __cplusplus
extern "C" {
#endif
	DLL_EXPORT uint32 LightSystem_LoadLight(const char* sceneLight);
	DLL_EXPORT uint32 LightSystem_AllocateLight(GameObjectTypeEnum lightType);
	DLL_EXPORT DirectionalLightComponent& LightSystem_GetDirectionalLight(uint directionalLightId);
	DLL_EXPORT PointLightComponent& LightSystem_GetPointLight(uint pointLightId);
#ifdef __cplusplus
}
#endif

