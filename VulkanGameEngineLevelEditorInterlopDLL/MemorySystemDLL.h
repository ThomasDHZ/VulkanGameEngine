#pragma once
#include <MemorySystem.h>

#ifdef __cplusplus
extern "C" {
#endif
    DLL_EXPORT MemoryLeakPtr MemoryLeakPtr_NewPtr(size_t size, size_t count, const char* file, int line, const char* type, const char* func, const char* notes);
    DLL_EXPORT void MemoryLeakPtr_DeletePtr(void* ptr);
    DLL_EXPORT void MemoryLeakPtr_ReportLeaks();
#ifdef __cplusplus
}
#endif
