#pragma once
#include "Platform.h"
#include "VulkanRenderer.h"

class GPUSystem
{
private:

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

	GPUSystem();
	~GPUSystem();
	DLL_EXPORT void StartUp();
	DLL_EXPORT bool CheckRayTracingCompatiblity(VkPhysicalDevice gpuDevice);
	DLL_EXPORT VkSampleCountFlagBits GetMaxUsableSampleCount(VkPhysicalDevice gpuDevice);
};
extern DLL_EXPORT GPUSystem gpuSystem;

