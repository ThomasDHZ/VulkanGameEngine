#include "MemoryPoolSystem.h"
#include "MeshSystem.h"
#include "BufferSystem.h"

MemoryPoolSystem& memoryPoolSystem = MemoryPoolSystem::Get();

void MemoryPoolSystem::AddSceneDataBuffer()
{
	VkPhysicalDeviceProperties props;
	vkGetPhysicalDeviceProperties(vulkanSystem.PhysicalDevice, &props);

	VkDeviceSize minOffsetAlignment = props.limits.minStorageBufferOffsetAlignment;
	VkDeviceSize maxStorageBufferRange = props.limits.maxStorageBufferRange;

	uint32_t maxSSBOsPerStage = props.limits.maxPerStageDescriptorStorageBuffers;

	VkDeviceSize totalBufferSize = 0;
	totalBufferSize += sizeof(MeshPropertiesBuffer) * 68;
	totalBufferSize += sizeof(MaterialProperitiesBuffer) * 112;
	totalBufferSize += sizeof(DirectionalLightBuffer) * 4;
	totalBufferSize += sizeof(PointLightBuffer) * 512;
	totalBufferSize += sizeof(mat4) * 68;
	totalBufferSize *= 1.5f;

	SceneDataBuffer.resize(totalBufferSize);
	bufferSystem.VMACreateDynamicBuffer(SceneDataBuffer.data(), SceneDataBuffer.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
}