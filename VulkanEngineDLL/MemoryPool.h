#pragma once
#include "BufferSystem.h"

struct MeshPropertiesStruct
{
	alignas(8)  size_t ShaderMaterialBufferIndex = 0;
	alignas(16) mat4   MeshTransform = mat4(1.0f);
};

template<typename T>
class MemoryPool
{
public:

	uint32		   BufferId = UINT32_MAX;
	Vector<T>	   CPUData;
	Vector<uint32> FreeIndices;
	uint32		   ActiveCount = 0;
	bool		   IsDirty = true;

private:
	void		   ResizeMemoryPool(uint newCapacity)
	{
		Vector<T> newCPUData(newCapacity);
		if (!CPUData.empty())
		{
			std::memcpy(newCPUData.data(), CPUData.data(), sizeof(T) * CPUData.size());
		}

		VkDeviceSize newSize = sizeof(T) * newCapacity;
		uint32 newBufferId = bufferSystem.VMACreateDynamicBuffer(nullptr, newSize, bufferSystem.FindVulkanBuffer(BufferId).BufferUsage);

		VulkanBuffer& newBuf = bufferSystem.FindVulkanBuffer(newBufferId);
		assert(newBuf.IsPersistentlyMapped && newBuf.BufferData);

		std::memcpy(newBuf.BufferData, newCPUData.data(), sizeof(T) * CPUData.size());
		bufferSystem.VMAUpdateDynamicBuffer(newBufferId, newCPUData.data(), sizeof(T) * CPUData.size(), 0);

		VkDescriptorBufferInfo descriptorBufferInfo
		{
			.buffer = bufferSystem.FindVulkanBuffer(ObjectDataPool.BufferId).Buffer,
			.offset = 0,
			.range = VK_WHOLE_SIZE
		};

		VkWriteDescriptorSet writeDescriptorSet = VkWriteDescriptorSet
		{
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet = globalBindlessSet,
			.dstBinding = 9,  // mesh properties slot
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			.pBufferInfo = &descriptorBufferInfo
		};
		vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);

		bufferSystem.DestroyBuffer(bufferSystem.FindVulkanBuffer(BufferId));
		BufferId = newBufferId;
		CPUData = std::move(newCPUData);
		IsDirty = true;
	}

public:

	MemoryPool()
	{

	}

	~MemoryPool()
	{
		/*	if (BufferId)
			{
				bufferSystem.DestroyBuffer(bufferSystem.FindVulkanBuffer(BufferId));
				BufferId = 0;
			}*/
	}

	void CreateMemoryPool(uint32 initialCapacity, VkBufferUsageFlags usage)
	{
		CPUData.resize(initialCapacity);
		ActiveCount = 0;

		VkDeviceSize size = sizeof(T) * initialCapacity;
		BufferId = bufferSystem.VMACreateDynamicBuffer(nullptr, size, usage);
		std::fill(CPUData.begin(), CPUData.end(), T{});
	}

	uint32 AllocateObject()
	{
		if (!FreeIndices.empty())
		{
			uint32 index = FreeIndices.back();
			FreeIndices.pop_back();

			CPUData[index] = T();
			if (index + 1 > ActiveCount)
			{
				ActiveCount = index + 1;
			}
			IsDirty = true;
			return index;
		}

		if (ActiveCount == CPUData.size())
		{
			ResizeMemoryPool(CPUData.capacity() ? CPUData.capacity() * 2 : 1024);
		}

		uint32 index = ActiveCount++;
		CPUData[index] = T();
		IsDirty = true;
		return index;
	}

	void FreeDataSlot(uint32 index)
	{
		FreeIndices.push_back(index);
		IsDirty = true;
	}

	T& Get(uint32 index)
	{
		return CPUData[index];
	}

	void MarkDirty() { IsDirty = true; }

	void UploadIfDirty()
	{
		if (IsDirty && BufferId)
		{
			VulkanBuffer& buffer = bufferSystem.FindVulkanBuffer(BufferId);
			assert(buffer.IsPersistentlyMapped && buffer.BufferData);

			VkDeviceSize uploadSize = sizeof(T) * ActiveCount;
			bufferSystem.VMAUpdateDynamicBuffer(BufferId, CPUData.data(), uploadSize, 0);

			IsDirty = false;
		}
	}
};