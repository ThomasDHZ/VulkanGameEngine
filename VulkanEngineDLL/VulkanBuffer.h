#pragma once
#include <vulkan/vulkan.h> 
#include <stdint.h>      
#include <stdbool.h>  
#include <iostream>
#include <memory>
#include <vector>
#include "Macro.h"
#include "Typedef.h"
#include "VulkanRenderer.h"

enum MeshTypeEnum
{
	Mesh_SpriteMesh,
	Mesh_LevelMesh,
	Mesh_LineMesh
};

enum BufferTypeEnum
{
	BufferType_Undefined,
	BufferType_UInt,
	BufferType_Mat4,
	BufferType_MaterialProperitiesBuffer,
	BufferType_SpriteInstanceStruct,
	BufferType_MeshPropertiesStruct,
	BufferType_SpriteMesh,
	BufferType_LevelLayerMesh,
	BufferType_Material,
	BufferType_Vertex2D
};

struct VulkanBuffer
{
	uint BufferId = 0;
	VkBuffer Buffer = VK_NULL_HANDLE;             
	VkBuffer StagingBuffer = VK_NULL_HANDLE;     
	VkDeviceMemory StagingBufferMemory = VK_NULL_HANDLE;
	VkDeviceMemory BufferMemory = VK_NULL_HANDLE;  
	VkDeviceSize BufferSize = 0;                   
	VkBufferUsageFlags BufferUsage = 0;             
	VkMemoryPropertyFlags BufferProperties = 0;    
	uint64_t BufferDeviceAddress = 0;              
	VkAccelerationStructureKHR BufferHandle = VK_NULL_HANDLE; 
	BufferTypeEnum  BufferType;              
	void* BufferData = nullptr;                    
	bool IsMapped = false;                         
	bool UsingStagingBuffer = false;       
};

#ifdef __cplusplus
extern "C" {
#endif
	DLL_EXPORT VulkanBuffer VulkanBuffer_CreateVulkanBuffer(const GraphicsRenderer& renderer, uint bufferId, VkDeviceSize bufferElementSize, uint bufferElementCount, BufferTypeEnum bufferTypeEnum, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, bool usingStagingBuffer);
	DLL_EXPORT VulkanBuffer VulkanBuffer_CreateVulkanBuffer2(const GraphicsRenderer& renderer, uint bufferId, void* bufferData, VkDeviceSize bufferElementSize, uint bufferElementCount, BufferTypeEnum bufferTypeEnum, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, bool usingStagingBuffer);
	DLL_EXPORT VulkanBuffer VulkanBuffer_CreateVulkanBuffer3(const GraphicsRenderer& renderer, VulkanBuffer& vulkanBuffer, uint bufferId, void* bufferData, VkDeviceSize bufferElementSize, uint bufferElementCount, BufferTypeEnum bufferTypeEnum, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, bool usingStagingBuffer);
	DLL_EXPORT void VulkanBuffer_UpdateBufferMemory(const GraphicsRenderer& renderer, VulkanBuffer& vulkanBuffer, void* bufferData, VkDeviceSize bufferElementSize, uint bufferElementCount);
	DLL_EXPORT VkResult VulkanBuffer_CopyBuffer(const GraphicsRenderer& renderer, VkBuffer* srcBuffer, VkBuffer* dstBuffer, VkDeviceSize size);
	DLL_EXPORT void VulkanBuffer_DestroyBuffer(const GraphicsRenderer& renderer, VulkanBuffer& vulkanBuffer);
#ifdef __cplusplus
}
#endif

void VulkanBuffer_UpdateBufferSize(const GraphicsRenderer& renderer, VulkanBuffer& vulkanBuffer, VkDeviceSize newBufferElementSize, uint32_t newBufferElementCount);
VkResult Buffer_UpdateBufferMemory(const GraphicsRenderer& renderState, VkDeviceMemory bufferMemory, void* dataToCopy, VkDeviceSize bufferSize);
void Buffer_CopyBufferMemory(const GraphicsRenderer& renderState, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize bufferSize);
VkResult Buffer_AllocateMemory(const GraphicsRenderer& renderState, VkBuffer* bufferData, VkDeviceMemory* bufferMemory, VkMemoryPropertyFlags properties);
void* Buffer_MapBufferMemory(const GraphicsRenderer& renderState, VkDeviceMemory bufferMemory, VkDeviceSize bufferSize, bool* isMapped);
VkResult Buffer_UnmapBufferMemory(const GraphicsRenderer& renderState, VkDeviceMemory bufferMemory, bool* isMapped);
VkResult Buffer_CreateBuffer(const GraphicsRenderer& renderState, VkBuffer* buffer, VkDeviceMemory* bufferMemory, void* bufferData, VkDeviceSize bufferSize, VkMemoryPropertyFlags properties, VkBufferUsageFlags usage);
VkResult Buffer_CreateStagingBuffer(const GraphicsRenderer& renderState, VkBuffer* stagingBuffer, VkBuffer* buffer, VkDeviceMemory* stagingBufferMemory, VkDeviceMemory* bufferMemory, void* bufferData, VkDeviceSize bufferSize, VkBufferUsageFlags bufferUsage, VkMemoryPropertyFlags properties);
VkResult Buffer_CopyBuffer(const GraphicsRenderer& renderState, VkBuffer* srcBuffer, VkBuffer* dstBuffer, VkDeviceSize size);
VkResult Buffer_UpdateBufferSize(const GraphicsRenderer& renderState, VkBuffer* buffer, VkDeviceMemory* bufferMemory, void* bufferData, VkDeviceSize oldBufferSize, VkDeviceSize newBufferSize, VkBufferUsageFlags bufferUsageFlags, VkMemoryPropertyFlags propertyFlags);
void Buffer_UpdateBufferData(const GraphicsRenderer& renderState, VkDeviceMemory* bufferMemory, void* dataToCopy, VkDeviceSize bufferSize);
void Buffer_UpdateStagingBufferData(const GraphicsRenderer& renderState, VkBuffer stagingBuffer, VkBuffer buffer, VkDeviceMemory* stagingBufferMemory, VkDeviceMemory* bufferMemory, void* dataToCopy, VkDeviceSize bufferSize);
VkResult Buffer_DestroyBuffer(const GraphicsRenderer& renderState, VkBuffer* buffer, VkBuffer* stagingBuffer, VkDeviceMemory* bufferMemory, VkDeviceMemory* stagingBufferMemory, void** bufferData, VkDeviceSize* bufferSize, VkBufferUsageFlags* bufferUsage, VkMemoryPropertyFlags* propertyFlags);
