#pragma once
#include "DLL.h"
#include <InputSystem.h>

#ifdef __cplusplus
extern "C" {
#endif
    DLL_EXPORT void                         InputSystem_Update(const float& deltaTime);

#ifdef __cplusplus
}
#endif