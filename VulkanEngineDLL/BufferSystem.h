#pragma once
#include "Platform.h"
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
	uint64 BufferDeviceAddress = 0;
	VkAccelerationStructureKHR BufferHandle = VK_NULL_HANDLE;
	BufferTypeEnum  BufferType;
	void* BufferData = nullptr;
	bool IsMapped = false;
	bool UsingStagingBuffer = false;
};

extern DLL_EXPORT int NextBufferId;

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

    void     CreateBuffer(VkBuffer* buffer, VkDeviceMemory* bufferMemory, void* bufferData, VkDeviceSize bufferSize, VkMemoryPropertyFlags properties, VkBufferUsageFlags usage);
    void     UpdateBufferSize(VulkanBuffer& vulkanBuffer, VkDeviceSize newBufferElementSize, uint32_t newBufferElementCount);
    void     UpdateBufferMemory(VkDeviceMemory bufferMemory, void* dataToCopy, VkDeviceSize bufferSize);
    void     CopyBufferMemory(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize bufferSize);
    void     AllocateMemory(VkBuffer* bufferData, VkDeviceMemory* bufferMemory, VkMemoryPropertyFlags properties, VkBufferUsageFlags usage);
    void*    MapBufferMemory(VkDeviceMemory bufferMemory, VkDeviceSize bufferSize, bool* isMapped);
    void     UnmapBufferMemory(VkDeviceMemory bufferMemory, bool* isMapped);
    void     UpdateBufferSize(VkBuffer* buffer, VkDeviceMemory* bufferMemory, void* bufferData, VkDeviceSize oldBufferSize, VkDeviceSize newBufferSize, VkBufferUsageFlags bufferUsageFlags, VkMemoryPropertyFlags propertyFlags);
    void     UpdateBufferData(VkDeviceMemory* bufferMemory, void* dataToCopy, VkDeviceSize bufferSize);
    void     UpdateStagingBufferData(VkBuffer stagingBuffer, VkBuffer buffer, VkDeviceMemory* stagingBufferMemory, VkDeviceMemory* bufferMemory, void* dataToCopy, VkDeviceSize bufferSize);

public:
    template <typename T>
    uint32 CreateVulkanBuffer(T& bufferData, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, bool usingStagingBuffer)
    {
        BufferTypeEnum bufferTypeEnum = GetBufferType<T>();
        VkDeviceSize bufferElementSize = sizeof(T);
        uint bufferElementCount = 1;

        int nextBufferId = ++NextBufferId;
        VulkanBufferMap[nextBufferId] = CreateVulkanBuffer(nextBufferId, static_cast<void*>(&bufferData), bufferElementSize, bufferElementCount, bufferTypeEnum, usage, properties, usingStagingBuffer);
        return nextBufferId;
    }

    template <typename T>
    uint32 CreateVulkanBuffer(Vector<T>& bufferData, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, bool usingStagingBuffer)
    {
        BufferTypeEnum bufferTypeEnum = GetBufferType<T>();
        VkDeviceSize bufferElementSize = sizeof(T);
        uint bufferElementCount = bufferData.size();

        int nextBufferId = ++NextBufferId;
        VulkanBufferMap[nextBufferId] = CreateVulkanBuffer(nextBufferId, bufferData.data(), bufferElementSize, bufferElementCount, bufferTypeEnum, usage, properties, usingStagingBuffer);
        return nextBufferId;
    }

    template <typename T>
    void UpdateBufferMemory(uint32 bufferId, T& bufferData)
    {
        BufferTypeEnum bufferTypeEnum = GetBufferType<T>();
        if (VulkanBufferMap[bufferId].BufferType != bufferTypeEnum)
            throw std::runtime_error("Buffer type doesn't match");

        VkDeviceSize bufferElementSize = sizeof(T);
        uint bufferElementCount = 1;

        UpdateBufferMemory(VulkanBufferMap[bufferId], static_cast<void*>(&bufferData), bufferElementSize, bufferElementCount);
    }

    template <typename T>
    void UpdateBufferMemory(uint32 bufferId, Vector<T>& bufferData)
    {
        BufferTypeEnum bufferTypeEnum = GetBufferType<T>();
        if (VulkanBufferMap[bufferId].BufferType != bufferTypeEnum)
            throw std::runtime_error("Buffer type doesn't match");

        VkDeviceSize bufferElementSize = sizeof(T);
        uint bufferElementCount = bufferData.size();

        UpdateBufferMemory(VulkanBufferMap[bufferId], bufferData.data(), bufferElementSize, bufferElementCount);
    }

    template <typename T>
    Vector<T> CheckBufferMemory(uint32 vulkanBufferId)
    {
        VulkanBuffer& vulkanBuffer = FindVulkanBuffer(vulkanBufferId);

        Vector<T> DataList;
        size_t dataListSize = vulkanBuffer.BufferSize / sizeof(T);

        void* data = MapBufferMemory(vulkanBuffer.BufferMemory, vulkanBuffer.BufferSize, &vulkanBuffer.IsMapped);
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
        UnmapBufferMemory(vulkanBuffer.BufferMemory, &vulkanBuffer.IsMapped);

        return DataList;
    }

    DLL_EXPORT VulkanBuffer                CreateVulkanBuffer(uint bufferId, VkDeviceSize bufferElementSize, uint bufferElementCount, BufferTypeEnum bufferTypeEnum, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, bool usingStagingBuffer);
    DLL_EXPORT VulkanBuffer                CreateVulkanBuffer(uint bufferId, void* bufferData, VkDeviceSize bufferElementSize, uint bufferElementCount, BufferTypeEnum bufferTypeEnum, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, bool usingStagingBuffer);
    DLL_EXPORT void                        CreateStagingBuffer(VkBuffer* stagingBuffer, VkBuffer* buffer, VkDeviceMemory* stagingBufferMemory, VkDeviceMemory* bufferMemory, void* bufferData, VkDeviceSize bufferSize, VkBufferUsageFlags bufferUsage, VkMemoryPropertyFlags properties);
    DLL_EXPORT void                        UpdateBufferMemory(VulkanBuffer& vulkanBuffer, void* bufferData, VkDeviceSize bufferElementSize, uint bufferElementCount);
    DLL_EXPORT void                        CopyBuffer(VkBuffer* srcBuffer, VkBuffer* dstBuffer, VkDeviceSize size);
    DLL_EXPORT void                        DestroyBuffer(VulkanBuffer& vulkanBuffer);
    DLL_EXPORT void                        DestroyAllBuffers();
    DLL_EXPORT VulkanBuffer&               FindVulkanBuffer(int id);
    DLL_EXPORT const Vector<VulkanBuffer>& VulkanBufferList();
};
extern DLL_EXPORT VulkanBufferSystem bufferSystem;

#ifdef __cplusplus
extern "C" {
#endif
	DLL_EXPORT VulkanBuffer VulkanBuffer_CreateVulkanBuffer(uint bufferId, VkDeviceSize bufferElementSize, uint bufferElementCount, BufferTypeEnum bufferTypeEnum, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, bool usingStagingBuffer);
	DLL_EXPORT VulkanBuffer VulkanBuffer_CreateVulkanBuffer2(uint bufferId, void* bufferData, VkDeviceSize bufferElementSize, uint bufferElementCount, BufferTypeEnum bufferTypeEnum, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, bool usingStagingBuffer);
	DLL_EXPORT void         VulkanBuffer_UpdateBufferMemory(VulkanBuffer& vulkanBuffer, void* bufferData, VkDeviceSize bufferElementSize, uint bufferElementCount);
	DLL_EXPORT void         VulkanBuffer_CopyBuffer(VkBuffer* srcBuffer, VkBuffer* dstBuffer, VkDeviceSize size);
	DLL_EXPORT void         VulkanBuffer_DestroyBuffer(VulkanBuffer& vulkanBuffer);
#ifdef __cplusplus
}
#endif