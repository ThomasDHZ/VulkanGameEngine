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

DLL_EXPORT int NextBufferId;

struct Vertex2D;
struct SpriteInstance;
struct MeshPropertiesStruct;
class VulkanBufferSystem
{
public:
    UnorderedMap<int, VulkanBuffer> VulkanBufferMap;

private:
    template <typename T>
    BufferTypeEnum GetBufferType()
    {
        if constexpr (std::is_same_v<T, uint32>)
        {
            return BufferType_UInt;
        }
        else if constexpr (std::is_same_v<T, mat4>)
        {
            return BufferType_Mat4;
        }
        else if constexpr (std::is_same_v<T, MeshPropertiesStruct>)
        {
            return BufferType_MeshPropertiesStruct;
        }
        else if constexpr (std::is_same_v<T, SpriteInstance>)
        {
            return BufferType_SpriteInstanceStruct;
        }
        else if constexpr (std::is_same_v<T, Vertex2D>)
        {
            return BufferType_Vertex2D;
        }
        else {
            throw std::runtime_error("Buffer type doesn't match");
        }
    }

    void VulkanBuffer_UpdateBufferSize(const GraphicsRenderer& renderer, VulkanBuffer& vulkanBuffer, VkDeviceSize newBufferElementSize, uint32_t newBufferElementCount);
    void Buffer_CopyBufferMemory(const GraphicsRenderer& renderState, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize bufferSize);
    VkResult Buffer_AllocateMemory(const GraphicsRenderer& renderState, VkBuffer* bufferData, VkDeviceMemory* bufferMemory, VkMemoryPropertyFlags properties);
    void* Buffer_MapBufferMemory(const GraphicsRenderer& renderState, VkDeviceMemory bufferMemory, VkDeviceSize bufferSize, bool* isMapped);
    VkResult Buffer_UnmapBufferMemory(const GraphicsRenderer& renderState, VkDeviceMemory bufferMemory, bool* isMapped);
    VkResult Buffer_UpdateBufferSize(const GraphicsRenderer& renderState, VkBuffer* buffer, VkDeviceMemory* bufferMemory, void* bufferData, VkDeviceSize oldBufferSize, VkDeviceSize newBufferSize, VkBufferUsageFlags bufferUsageFlags, VkMemoryPropertyFlags propertyFlags);
    void Buffer_UpdateBufferData(const GraphicsRenderer& renderState, VkDeviceMemory* bufferMemory, void* dataToCopy, VkDeviceSize bufferSize);
    void Buffer_UpdateStagingBufferData(const GraphicsRenderer& renderState, VkBuffer stagingBuffer, VkBuffer buffer, VkDeviceMemory* stagingBufferMemory, VkDeviceMemory* bufferMemory, void* dataToCopy, VkDeviceSize bufferSize);
    VkResult Buffer_DestroyBuffer(const GraphicsRenderer& renderState, VkBuffer* buffer, VkBuffer* stagingBuffer, VkDeviceMemory* bufferMemory, VkDeviceMemory* stagingBufferMemory, void** bufferData, VkDeviceSize* bufferSize, VkBufferUsageFlags* bufferUsage, VkMemoryPropertyFlags* propertyFlags);
    VkResult VulkanBuffer_CopyBuffer(const GraphicsRenderer& renderer, VkBuffer* srcBuffer, VkBuffer* dstBuffer, VkDeviceSize size);
    void VulkanBuffer_DestroyBuffer(const GraphicsRenderer& renderer, VulkanBuffer& vulkanBuffer);

public:
    template <typename T>
    uint32 CreateVulkanBuffer(const GraphicsRenderer& renderer, T& bufferData, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, bool usingStagingBuffer)
    {
        BufferTypeEnum bufferTypeEnum = GetBufferType<T>();
        VkDeviceSize bufferElementSize = sizeof(T);
        uint bufferElementCount = 1;

        int nextBufferId = ++NextBufferId;
        VulkanBufferMap[nextBufferId] = VulkanBuffer_CreateVulkanBuffer2(renderer, nextBufferId, static_cast<void*>(&bufferData), bufferElementSize, bufferElementCount, bufferTypeEnum, usage, properties, usingStagingBuffer);
        return nextBufferId;
    }

    template <typename T>
    uint32 CreateVulkanBuffer(const GraphicsRenderer& renderer, Vector<T>& bufferData, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, bool usingStagingBuffer)
    {
        BufferTypeEnum bufferTypeEnum = GetBufferType<T>();
        VkDeviceSize bufferElementSize = sizeof(T);
        uint bufferElementCount = bufferData.size();

        int nextBufferId = ++NextBufferId;
        VulkanBufferMap[nextBufferId] = VulkanBuffer_CreateVulkanBuffer2(renderer, nextBufferId, bufferData.data(), bufferElementSize, bufferElementCount, bufferTypeEnum, usage, properties, usingStagingBuffer);
        return nextBufferId;
    }

    template <typename T>
    void UpdateBufferMemory(const GraphicsRenderer& renderer, uint32 bufferId, T& bufferData)
    {
        BufferTypeEnum bufferTypeEnum = GetBufferType<T>();
        if (VulkanBufferMap[bufferId].BufferType != bufferTypeEnum)
            throw std::runtime_error("Buffer type doesn't match");

        VkDeviceSize bufferElementSize = sizeof(T);
        uint bufferElementCount = 1;

        VulkanBuffer_UpdateBufferMemory(renderer, VulkanBufferMap[bufferId], static_cast<void*>(&bufferData), bufferElementSize, bufferElementCount);
    }

    template <typename T>
    void UpdateBufferMemory(const GraphicsRenderer& renderer, uint32 bufferId, Vector<T>& bufferData)
    {
        BufferTypeEnum bufferTypeEnum = GetBufferType<T>();
        if (VulkanBufferMap[bufferId].BufferType != bufferTypeEnum)
            throw std::runtime_error("Buffer type doesn't match");

        VkDeviceSize bufferElementSize = sizeof(T);
        uint bufferElementCount = bufferData.size();

        VulkanBuffer_UpdateBufferMemory(renderer, VulkanBufferMap[bufferId], bufferData.data(), bufferElementSize, bufferElementCount);
    }

    template <typename T>
    Vector<T> CheckBufferMemory(GraphicsRenderer& renderer, uint32 vulkanBufferId)
    {
        VulkanBuffer& vulkanBuffer = FindVulkanBuffer(vulkanBufferId);

        Vector<T> DataList;
        size_t dataListSize = vulkanBuffer.BufferSize / sizeof(T);

        void* data = Buffer_MapBufferMemory(renderer, vulkanBuffer.BufferMemory, vulkanBuffer.BufferSize, &vulkanBuffer.IsMapped);
        if (data == nullptr)
        {
            std::cerr << "Failed to map buffer memory\n";
            return DataList;
        }

        char* newPtr = static_cast<char*>(data);
        for (size_t x = 0; x < dataListSize; ++x)
        {
            DataList.emplace_back(*reinterpret_cast<T*>(newPtr));
            newPtr += sizeof(T);
        }
        Buffer_UnmapBufferMemory(renderer, vulkanBuffer.BufferMemory, &vulkanBuffer.IsMapped);

        return DataList;
    }

    DLL_EXPORT VulkanBuffer VulkanBuffer_CreateVulkanBuffer(const GraphicsRenderer& renderer, uint bufferId, VkDeviceSize bufferElementSize, uint bufferElementCount, BufferTypeEnum bufferTypeEnum, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, bool usingStagingBuffer);
    DLL_EXPORT VulkanBuffer VulkanBuffer_CreateVulkanBuffer2(const GraphicsRenderer& renderer, uint bufferId, void* bufferData, VkDeviceSize bufferElementSize, uint bufferElementCount, BufferTypeEnum bufferTypeEnum, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, bool usingStagingBuffer);
    DLL_EXPORT VulkanBuffer VulkanBuffer_CreateVulkanBuffer3(const GraphicsRenderer& renderer, VulkanBuffer& vulkanBuffer, uint bufferId, void* bufferData, VkDeviceSize bufferElementSize, uint bufferElementCount, BufferTypeEnum bufferTypeEnum, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, bool usingStagingBuffer);
    DLL_EXPORT VkResult Buffer_CreateBuffer(const GraphicsRenderer& renderState, VkBuffer* buffer, VkDeviceMemory* bufferMemory, void* bufferData, VkDeviceSize bufferSize, VkMemoryPropertyFlags properties, VkBufferUsageFlags usage);
    DLL_EXPORT VkResult Buffer_CreateStagingBuffer(const GraphicsRenderer& renderState, VkBuffer* stagingBuffer, VkBuffer* buffer, VkDeviceMemory* stagingBufferMemory, VkDeviceMemory* bufferMemory, void* bufferData, VkDeviceSize bufferSize, VkBufferUsageFlags bufferUsage, VkMemoryPropertyFlags properties);
    DLL_EXPORT VulkanBuffer& FindVulkanBuffer(int id);
    DLL_EXPORT VkResult Buffer_UpdateBufferMemory(const GraphicsRenderer& renderState, VkDeviceMemory bufferMemory, void* dataToCopy, VkDeviceSize bufferSize);
    DLL_EXPORT void VulkanBuffer_UpdateBufferMemory(const GraphicsRenderer& renderer, VulkanBuffer& vulkanBuffer, void* bufferData, VkDeviceSize bufferElementSize, uint bufferElementCount);
    DLL_EXPORT VkResult Buffer_CopyBuffer(const GraphicsRenderer& renderState, VkBuffer* srcBuffer, VkBuffer* dstBuffer, VkDeviceSize size);
    DLL_EXPORT void DestroyBuffer(const GraphicsRenderer& renderer, int vulkanBufferId);
    DLL_EXPORT void DestroyAllBuffers(const GraphicsRenderer& renderer);
    DLL_EXPORT const Vector<VulkanBuffer>& VulkanBufferList();
};
DLL_EXPORT VulkanBufferSystem bufferSystem;

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