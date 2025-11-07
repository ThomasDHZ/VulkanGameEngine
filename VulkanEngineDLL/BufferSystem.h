#pragma once
#include "Platform.h"
#include "VulkanBuffer.h"
#include "MeshSystem.h"
#include "MaterialSystem.h"

DLL_EXPORT int NextBufferId;

struct Vertex2D;
struct SpriteInstance;
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

    DLL_EXPORT VulkanBuffer& FindVulkanBuffer(int id);
    DLL_EXPORT const Vector<VulkanBuffer>& VulkanBufferList();

    DLL_EXPORT void DestroyBuffer(const GraphicsRenderer& renderer, int vulkanBufferId);
    DLL_EXPORT void DestroyAllBuffers(const GraphicsRenderer& renderer);

    static VkResult CopyBuffer(const GraphicsRenderer& renderer, VkBuffer* srcBuffer, VkBuffer* dstBuffer, VkDeviceSize size)
    {
        return Buffer_CopyBuffer(renderer, srcBuffer, dstBuffer, size);
    }
};
DLL_EXPORT VulkanBufferSystem bufferSystem;