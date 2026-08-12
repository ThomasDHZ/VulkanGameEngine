#pragma once
#include <Platform.h>
#include "LevelSystem.h"

#ifdef __cplusplus
extern "C" {
#endif
    DLL_EXPORT void                           LevelSystem_LoadLevel(const char* levelPath);
    DLL_EXPORT void                           LevelSystem_Update(const float& deltaTime);
#ifdef __cplusplus
}
#endif