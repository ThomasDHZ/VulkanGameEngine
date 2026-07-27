#include "MemorySystemDLL.h"

extern "C"
{

    void MemoryLeakPtr_DeletePtr(void* memoryLeakPtr)
    {
        memorySystem.DeletePtr(memoryLeakPtr);
    }

    void MemoryLeakPtr_ReportLeaks()
    {
        memorySystem.ReportLeaks();
    }
}