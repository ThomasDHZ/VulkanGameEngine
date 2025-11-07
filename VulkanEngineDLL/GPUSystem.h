#pragma once
#include "Platform.h"
#include "VulkanRenderer.h"

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

class GPUSystem
{
private:
	bool CheckRayTracingCompatiblity(VkPhysicalDevice gpuDevice)
	{
		return GPUSystem_CheckRayTracingCompatiblity(gpuDevice);
	}

	VkSampleCountFlagBits GetMaxUsableSampleCount(VkPhysicalDevice gpuDevice)
	{
		return GPUSystem_GetMaxUsableSampleCount(gpuDevice);
	}

public:
	Vector<String> FeatureList;
	Vector<const char*> DeviceExtensionList;
	VkPhysicalDeviceFeatures PhysicalDeviceFeatures;
	VkPhysicalDeviceProperties PhysicalDeviceProperties;
	VkPhysicalDeviceLimits PhysicalDeviceLimits;
	VkPhysicalDeviceAccelerationStructureFeaturesKHR AccelerationStructureFeatures;
	VkPhysicalDeviceRayTracingPipelineFeaturesKHR RayTracingPipelineFeatures;
	VkPhysicalDeviceRayTracingPipelinePropertiesKHR  RayTracingPipelineProperties;
	VkSampleCountFlagBits MaxSampleCount;
	bool RayTracingFeature;

	GPUSystem()
	{

	}

	~GPUSystem()
	{

	}

	void StartUp()
	{
		GPUSystem_StartUp();
	}
};
DLL_EXPORT GPUSystem gpuSystem;

