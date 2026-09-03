#pragma once
#include "DLL.h"
#include <RenderSystem.h>
#include "CameraSystem.h"

#ifdef __cplusplus
extern "C" {
#endif
    DLL_EXPORT Camera*               CameraSystem_UpdateActiveCamera();
#ifdef __cplusplus
}
#endif