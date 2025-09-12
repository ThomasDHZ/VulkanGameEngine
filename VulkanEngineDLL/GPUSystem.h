#pragma once
#include <vulkan/vulkan.h>
#include "Typedef.h"
#include "VulkanRenderer.h"

class GPUSystem
{
private:
	void CheckRayTracingCompatiblity(VkPhysicalDevice GPUDevice)
	{
		if (!RayTracingFeature)
		{
			VkPhysicalDeviceAccelerationStructureFeaturesKHR AccelerationStructureFeatures{};
			AccelerationStructureFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;

			VkPhysicalDeviceRayTracingPipelineFeaturesKHR RayTracingPipelineFeatures{};
			RayTracingPipelineFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
			RayTracingPipelineFeatures.pNext = &AccelerationStructureFeatures;

			VkPhysicalDeviceFeatures2 DeviceFeatures2{};
			DeviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
			DeviceFeatures2.pNext = &RayTracingPipelineFeatures;
			vkGetPhysicalDeviceFeatures2(GPUDevice, &DeviceFeatures2);

			if (RayTracingPipelineFeatures.rayTracingPipeline == VK_TRUE &&
				AccelerationStructureFeatures.accelerationStructure == VK_TRUE)
			{
				if (std::find(FeatureList.begin(), FeatureList.end(), VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME) != FeatureList.end() &&
					std::find(FeatureList.begin(), FeatureList.end(), VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME) != FeatureList.end())
				{
					RayTracingFeature = true;
					DeviceExtensionList.emplace_back(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
					DeviceExtensionList.emplace_back(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
				}
				else
				{
					RayTracingFeature = false;
				}
			}
			else
			{
				std::cout << "GPU/MotherBoard isn't ray tracing compatible." << std::endl;
			}
		}
	}

	VkSampleCountFlagBits GetMaxUsableSampleCount(VkPhysicalDevice GPUDevice)
	{
		VkSampleCountFlags counts = PhysicalDeviceLimits.framebufferColorSampleCounts & PhysicalDeviceLimits.framebufferDepthSampleCounts;
		if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
		if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
		if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
		if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
		if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
		if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }

		return VK_SAMPLE_COUNT_1_BIT;
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

	void StartUp(GraphicsRenderer& renderer)
	{
		AccelerationStructureFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;

		RayTracingPipelineFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
		RayTracingPipelineFeatures.pNext = &AccelerationStructureFeatures;

		VkPhysicalDeviceFeatures2 PhysicalDeviceFeatures2{};
		PhysicalDeviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		PhysicalDeviceFeatures2.pNext = &RayTracingPipelineFeatures;
		vkGetPhysicalDeviceFeatures2(renderer.PhysicalDevice, &PhysicalDeviceFeatures2);
		PhysicalDeviceFeatures = PhysicalDeviceFeatures2.features;

		vkGetPhysicalDeviceProperties(renderer.PhysicalDevice, &PhysicalDeviceProperties);
		PhysicalDeviceLimits = PhysicalDeviceProperties.limits;

		RayTracingPipelineProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
		VkPhysicalDeviceProperties2 PhysicalDeviceProperties{};
		PhysicalDeviceProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
		PhysicalDeviceProperties.pNext = &RayTracingPipelineProperties;
		vkGetPhysicalDeviceProperties2(renderer.PhysicalDevice, &PhysicalDeviceProperties);

		MaxSampleCount = GetMaxUsableSampleCount(renderer.PhysicalDevice);
	}
};
DLL_EXPORT GPUSystem gpuSystem;

