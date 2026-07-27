#pragma once
#include <MemorySystem.h>

#ifdef __cplusplus
extern "C" {
#endif
    DLL_EXPORT void MemoryLeakPtr_DeletePtr(void* ptr);
    DLL_EXPORT void MemoryLeakPtr_ReportLeaks();
#ifdef __cplusplus
}
#endif
