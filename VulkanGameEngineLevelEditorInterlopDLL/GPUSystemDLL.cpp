#include "pch.h"
#include "GPUSystemDLL.h"

void GPUSystem_StartUp()
{
	gpuSystem.StartUp();
}

bool GPUSystem_CheckRayTracingCompatiblity(VkPhysicalDevice gpuDevice)
{
	return gpuSystem.CheckRayTracingCompatiblity(gpuDevice);
}

VkSampleCountFlagBits GPUSystem_GetMaxUsableSampleCount(VkPhysicalDevice GPUDevice)
{
	return gpuSystem.GetMaxUsableSampleCount(GPUDevice);
}
