#pragma once
#include <InputSystem.h>
#include "DLL.h"

#ifdef __cplusplus
extern "C" {
#endif
    DLL_EXPORT void                         InputSystem_Update(const float& deltaTime);

#ifdef __cplusplus
}
#endif