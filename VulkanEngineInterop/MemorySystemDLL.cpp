#include "MemorySystemDLL.h"

void MemorySystem_DeletePtr(void* ptr)
{
    memorySystem.DeletePtr(ptr);
}

void MemorySystem_ReportLeaks()
{
    memorySystem.ReportLeaks();
}
