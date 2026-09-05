#pragma once
#include "DLL.h"
#include <LightSystem.h>

#ifdef __cplusplus
extern "C" {
#endif
	DLL_EXPORT DirectionalLightComponent& LightSystem_GetDirectionalLight(uint directionalLightId);
	DLL_EXPORT PointLightComponent& LightSystem_GetPointLight(uint pointLightId);
	DLL_EXPORT uint LightSystem_FindDirectionalLightIndex(void* ptr);
	DLL_EXPORT uint LightSystem_FindPointLightIndex(void* ptr);
#ifdef __cplusplus
}
#endif