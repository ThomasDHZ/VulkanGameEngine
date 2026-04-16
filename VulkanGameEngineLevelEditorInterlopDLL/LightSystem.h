#pragma once
#include <LightSystem.h>

#ifdef __cplusplus
extern "C" {
#endif
	DLL_EXPORT uint32 LightSystem_LoadLight(const char* sceneLight);
	DLL_EXPORT uint32 LightSystem_AllocateLight(LightTypeEnum lightType);
	DLL_EXPORT DirectionalLight& LightSystem_GetDirectionalLight(uint directionalLightId);
	DLL_EXPORT PointLight& LightSystem_GetPointLight(uint pointLightId);
#ifdef __cplusplus
}
#endif

