#pragma once
#include "Platform.h"
#include "VulkanSystem.h"

class GPUSystem
{
public:
	static GPUSystem& Get();

private:
	GPUSystem() = default;
	~GPUSystem() = default;
	GPUSystem(const GPUSystem&) = delete;
	GPUSystem& operator=(const GPUSystem&) = delete;
	GPUSystem(GPUSystem&&) = delete;
	GPUSystem& operator=(GPUSystem&&) = delete;

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

	DLL_EXPORT void StartUp();
	DLL_EXPORT bool CheckRayTracingCompatiblity(VkPhysicalDevice gpuDevice);
	DLL_EXPORT VkSampleCountFlagBits GetMaxUsableSampleCount(VkPhysicalDevice gpuDevice);
};
extern DLL_EXPORT GPUSystem& gpuSystem;
inline GPUSystem& GPUSystem::Get()
{
	static GPUSystem instance;
	return instance;
}

