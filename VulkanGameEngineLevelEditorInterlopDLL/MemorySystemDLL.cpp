#include "MemorySystemDLL.h"

extern "C"
{
    MemoryLeakPtr MemoryLeakPtr_NewPtr(size_t memorySize, size_t elementCount, const char* file, int line, const char* type, const char* func, const char* notes)
    {
        return memorySystem.NewPtr(memorySize, elementCount, file, line, type, func, notes);
    }

    void MemoryLeakPtr_DeletePtr(void* memoryLeakPtr)
    {
        memorySystem.DeletePtr(memoryLeakPtr);
    }

    void MemoryLeakPtr_ReportLeaks()
    {
        memorySystem.ReportLeaks();
    }
}