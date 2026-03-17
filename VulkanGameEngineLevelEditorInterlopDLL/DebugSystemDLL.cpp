#include "DebugSystemDLL.h"

void DebugSystem_SetRootDirectory(const char* engineRoot)
{
    debugSystem.SetRootDirectory(String(engineRoot));
}