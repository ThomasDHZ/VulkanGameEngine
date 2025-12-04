#pragma once
#include <GPUSystem.h>

#ifdef __cplusplus
extern "C"
{
#endif
	DLL_EXPORT bool GPUSystem_CheckRayTracingCompatiblity(VkPhysicalDevice gpuDevice);
	DLL_EXPORT VkSampleCountFlagBits GPUSystem_GetMaxUsableSampleCount(VkPhysicalDevice GPUDevice);
	DLL_EXPORT void GPUSystem_StartUp();
#ifdef __cplusplus
}
#endif

