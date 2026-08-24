#pragma once
#include "DLL.h"
#include <MemorySystem.h>

#ifdef __cplusplus
extern "C" {
#endif
    DLL_EXPORT void MemorySystem_DeletePtr(void* ptr);
#ifdef __cplusplus
}
#endif
