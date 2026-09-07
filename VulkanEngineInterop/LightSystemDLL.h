#pragma once
#include "DLL.h"
#include <LightSystem.h>

#ifdef __cplusplus
extern "C" {
#endif
	DLL_EXPORT DirectionalLight& LightSystem_GetDirectionalLight(uint directionalLightId);
	DLL_EXPORT PointLight& LightSystem_GetPointLight(uint pointLightId);
#ifdef __cplusplus
}
#endif