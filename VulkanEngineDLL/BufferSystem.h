#pragma once
#include "Platform.h"
#include "VulkanSystem.h"
#include <vk_mem_alloc.h>


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
	BufferType_Vertex2D,
    BufferType_VertexSkyBox
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
    VmaAllocation Allocation = VK_NULL_HANDLE;
	void* BufferData = nullptr;
	bool IsMapped = false;
	bool UsingStagingBuffer = false;
    bool IsPersistentlyMapped = false;
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

public:
    UnorderedMap<int, VulkanBuffer> VulkanBufferMap;

private:
    void     CopyBufferMemory(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize bufferSize);

public:
    VmaAllocator			vmaAllocator;

    template <typename T>
    uint32 CreateVulkanBuffer(T& bufferData, VkBufferUsageFlags shaderUsageFlags, bool usingStagingBuffer)
    {
        if (usingStagingBuffer)
        {
            return CreateStaticVulkanBuffer(static_cast<void*>(&bufferData), sizeof(T), shaderUsageFlags);
        }
        else
        {
            return CreateDynamicBuffer(static_cast<void*>(&bufferData), sizeof(T), shaderUsageFlags);
        }
    }

    template <typename T>
    uint32 CreateVulkanBuffer(Vector<T>& bufferData, VkBufferUsageFlags shaderUsageFlags, bool usingStagingBuffer)
    {
        VkDeviceSize bufferSize = sizeof(T) * bufferData.size();
        if (bufferData.empty())
        {
            bufferSize = sizeof(T) * 64;
        }

        if (usingStagingBuffer)
        {
            return CreateStaticVulkanBuffer(bufferData.data(), bufferSize, shaderUsageFlags);
        }
        else
        {
            return CreateDynamicBuffer(bufferData.data(), bufferSize, shaderUsageFlags);
        }
    }

    template <typename T>
    void UpdateBufferMemory(uint32 bufferId, T& bufferData)
    {
        UpdateDynamicBuffer(bufferId, static_cast<void*>(&bufferData), sizeof(T));
    }

    template <typename T>
    void UpdateBufferMemory(uint32 bufferId, Vector<T>& bufferData)
    {
        UpdateDynamicBuffer(bufferId, bufferData.data(), sizeof(T) * bufferData.size());
    }

    template <typename T>
    Vector<T> CheckBufferMemory(uint32 vulkanBufferId)
    {
        VulkanBuffer& vulkanBuffer = FindVulkanBuffer(vulkanBufferId);

        Vector<T> DataList;
   /*     size_t dataListSize = vulkanBuffer.BufferSize / sizeof(T);

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

    DLL_EXPORT uint32                      CreateStaticVulkanBuffer(const void* srcData, VkDeviceSize size, VkBufferUsageFlags shaderUsageFlags, VkDeviceSize offset = 0);
    DLL_EXPORT uint32                      CreateDynamicBuffer(const void* srcData, VkDeviceSize size, VkBufferUsageFlags usageFlags);
    DLL_EXPORT void                        UpdateDynamicBuffer(uint32 bufferId, const void* data, VkDeviceSize size, VkDeviceSize offset = 0);
    DLL_EXPORT void                        CopyBuffer(VkBuffer* srcBuffer, VkBuffer* dstBuffer, VkDeviceSize size, VkBufferUsageFlags shaderUsageFlags, VkDeviceSize offset = 0);
    DLL_EXPORT void                        DestroyBuffer(VulkanBuffer& vulkanBuffer);
    DLL_EXPORT void                        DestroyAllBuffers();
    DLL_EXPORT VulkanBuffer&               FindVulkanBuffer(int id);
    DLL_EXPORT const Vector<VulkanBuffer>& VulkanBufferList();
};
extern DLL_EXPORT VulkanBufferSystem& bufferSystem;
inline VulkanBufferSystem& VulkanBufferSystem::Get()
{
    static VulkanBufferSystem instance;
    return instance;
}