#pragma once
#include "Platform.h"
#include "VulkanSystem.h"

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

    // VMA allocations
    VmaAllocation Allocation = VK_NULL_HANDLE;
    VmaAllocation StagingAllocation = VK_NULL_HANDLE;

    // Persistent mapping (only for HOST_VISIBLE buffers)
    void* MappedData = nullptr;
    bool  IsPersistentlyMapped = false;

    VkDeviceSize BufferSize = 0;
    VkBufferUsageFlags BufferUsage = 0;
    VkMemoryPropertyFlags BufferProperties = 0;
    uint64 BufferDeviceAddress = 0;
    BufferTypeEnum BufferType;
    bool UsingStagingBuffer = false;
};

extern DLL_EXPORT int NextBufferId;

struct Vertex2D;
struct SpriteInstance;
struct MeshPropertiesStruct;
class VulkanBufferSystem
{
public:
    static VulkanBufferSystem& Get();

private:
    VulkanBufferSystem() = default;
    ~VulkanBufferSystem() = default;
    VulkanBufferSystem(const VulkanBufferSystem&) = delete;
    VulkanBufferSystem& operator=(const VulkanBufferSystem&) = delete;

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

    void CopyBufferMemory(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize bufferSize);

public:
    UnorderedMap<int, VulkanBuffer> VulkanBufferMap;

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

        UpdateBufferData(VulkanBufferMap[bufferId], static_cast<void*>(&bufferData), bufferElementSize);
    }

    template <typename T>
    void UpdateBufferMemory(uint32 bufferId, Vector<T>& bufferData)
    {
        BufferTypeEnum bufferTypeEnum = GetBufferType<T>();
        if (VulkanBufferMap[bufferId].BufferType != bufferTypeEnum)
            throw std::runtime_error("Buffer type doesn't match");

        VkDeviceSize bufferElementSize = sizeof(T);
        uint bufferElementCount = bufferData.size();

        UpdateBufferMemory(VulkanBufferMap[bufferId], bufferData.data(), bufferElementSize);
    }

    template <typename T>
    Vector<T> CheckBufferMemory(uint32 vulkanBufferId)
    {
        VulkanBuffer& vulkanBuffer = FindVulkanBuffer(vulkanBufferId);

        Vector<T> DataList;
        /*size_t dataListSize = vulkanBuffer.BufferSize / sizeof(T);

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
        UnmapBufferMemory(vulkanBuffer.BufferMemory, &vulkanBuffer.IsMapped);*/

        return DataList;
    }

    DLL_EXPORT VulkanBuffer CreateVulkanBuffer(uint bufferId, void* bufferData, VkDeviceSize bufferElementSize, uint bufferElementCount, BufferTypeEnum bufferTypeEnum, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, bool usingStagingBuffer);
    DLL_EXPORT void ResizeBuffer(VulkanBuffer& vulkanBuffer, VkDeviceSize newSize, void* newData);
    DLL_EXPORT void CopyBuffer(VkBuffer* srcBuffer, VkBuffer* dstBuffer, VkDeviceSize size);
    DLL_EXPORT void UpdateBufferMemory(VulkanBuffer& vulkanBuffer, void* bufferData, VkDeviceSize bufferSize);
    DLL_EXPORT VulkanBuffer& FindVulkanBuffer(int id);
    DLL_EXPORT void DestroyBuffer(VulkanBuffer& vulkanBuffer);
};
extern DLL_EXPORT VulkanBufferSystem& bufferSystem;
inline VulkanBufferSystem& VulkanBufferSystem::Get()
{
    static VulkanBufferSystem instance;
    return instance;
}