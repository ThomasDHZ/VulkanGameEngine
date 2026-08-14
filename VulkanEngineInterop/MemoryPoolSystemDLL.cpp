#include "MemoryPoolSystemDLL.h"

void MemoryPoolSystem_StartUp()
{
	memoryPoolSystem.StartUp();
}

void MemoryPoolSystem_Update()
{
	memoryPoolSystem.UpdateMemoryPool();
}
