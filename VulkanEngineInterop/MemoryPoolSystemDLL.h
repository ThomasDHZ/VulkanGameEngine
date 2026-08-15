#pragma once
#include "DLL.h" 
#include <MemoryPoolSystem.h>

#ifdef __cplusplus
extern "C" {
#endif
	DLL_EXPORT void											 MemoryPoolSystem_StartUp();
	DLL_EXPORT void											 MemoryPoolSystem_Update();
#ifdef __cplusplus
}
#endif