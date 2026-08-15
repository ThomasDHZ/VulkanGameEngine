#pragma once
#include "DLL.h" 
#include <MeshSystem.h>

#ifdef __cplusplus
extern "C" {
#endif
    DLL_EXPORT void                   MeshSystem_Update(const float& deltaTime);
#ifdef __cplusplus
}
#endif