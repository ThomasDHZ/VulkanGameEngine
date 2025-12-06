#include "GPUSystem.h"
GPUSystem gpuSystem = GPUSystem();

bool GPUSystem::CheckRayTracingCompatiblity(VkPhysicalDevice gpuDevice)
{
	if (!gpuSystem.RayTracingFeature)
	{
		VkPhysicalDeviceAccelerationStructureFeaturesKHR AccelerationStructureFeatures{};
		AccelerationStructureFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;

		VkPhysicalDeviceRayTracingPipelineFeaturesKHR RayTracingPipelineFeatures{};
		RayTracingPipelineFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
		RayTracingPipelineFeatures.pNext = &AccelerationStructureFeatures;

		VkPhysicalDeviceFeatures2 DeviceFeatures2{};
		DeviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		DeviceFeatures2.pNext = &RayTracingPipelineFeatures;
		vkGetPhysicalDeviceFeatures2(gpuDevice, &DeviceFeatures2);

		if (RayTracingPipelineFeatures.rayTracingPipeline == VK_TRUE &&
			AccelerationStructureFeatures.accelerationStructure == VK_TRUE)
		{
			if (std::find(gpuSystem.FeatureList.begin(), gpuSystem.FeatureList.end(), VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME) != gpuSystem.FeatureList.end() &&
				std::find(gpuSystem.FeatureList.begin(), gpuSystem.FeatureList.end(), VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME) != gpuSystem.FeatureList.end())
			{
				gpuSystem.RayTracingFeature = true;
				gpuSystem.DeviceExtensionList.emplace_back(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
				gpuSystem.DeviceExtensionList.emplace_back(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
			}
			else
			{
				gpuSystem.RayTracingFeature = false;
			}
		}
		else
		{
			std::cout << "GPU/MotherBoard isn't ray tracing compatible." << std::endl;
		}
	}

	return gpuSystem.RayTracingFeature;
}

VkSampleCountFlagBits GPUSystem::GetMaxUsableSampleCount(VkPhysicalDevice gpuDevice)
{
	VkSampleCountFlags counts = gpuSystem.PhysicalDeviceLimits.framebufferColorSampleCounts & gpuSystem.PhysicalDeviceLimits.framebufferDepthSampleCounts;
	if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
	if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
	if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
	if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
	if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
	if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }
	return VK_SAMPLE_COUNT_1_BIT;
}

GPUSystem::GPUSystem()
{
}

GPUSystem::~GPUSystem()
{
}

void GPUSystem::StartUp()
{
	gpuSystem.AccelerationStructureFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;

	gpuSystem.RayTracingPipelineFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
	gpuSystem.RayTracingPipelineFeatures.pNext = &gpuSystem.AccelerationStructureFeatures;

	VkPhysicalDeviceFeatures2 PhysicalDeviceFeatures2{};
	PhysicalDeviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	PhysicalDeviceFeatures2.pNext = &gpuSystem.RayTracingPipelineFeatures;
	vkGetPhysicalDeviceFeatures2(vulkanSystem.PhysicalDevice, &PhysicalDeviceFeatures2);
	gpuSystem.PhysicalDeviceFeatures = PhysicalDeviceFeatures2.features;

	vkGetPhysicalDeviceProperties(vulkanSystem.PhysicalDevice, &gpuSystem.PhysicalDeviceProperties);
	gpuSystem.PhysicalDeviceLimits = gpuSystem.PhysicalDeviceProperties.limits;

	gpuSystem.RayTracingPipelineProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
	VkPhysicalDeviceProperties2 PhysicalDeviceProperties{};
	PhysicalDeviceProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
	PhysicalDeviceProperties.pNext = &gpuSystem.RayTracingPipelineProperties;
	vkGetPhysicalDeviceProperties2(vulkanSystem.PhysicalDevice, &PhysicalDeviceProperties);

	gpuSystem.MaxSampleCount = gpuSystem.GetMaxUsableSampleCount(vulkanSystem.PhysicalDevice);
}
