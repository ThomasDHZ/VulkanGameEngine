//#pragma once
//#include "BufferSystem.h"
//#include <vector>
//#include "RenderSystem.h"
//#include "JsonStruct.h"
//
//struct MemoryPoolBufferHeader2
//{
//    //CPU side: in bytes
//    //GPU side: by index (bytes/4)
//    //GPU data array: by index (bytes/4)
//    uint ObjectOffset;
//    uint ObjectCount;
//    uint ObjectSize;
//};
//
//template<typename T>
//class MemoryPool
//{
//public:
//    uint32 BufferId = UINT32_MAX;
//    Vector<T> ObjectDataPool;
//    Vector<uint32_t> FreeIndices;
//    uint32 ActiveCount = UINT32_MAX;
//    Vector<byte> IsActive;         // 0 = inactive, 1 = active
//    bool IsDirty = true;
//
//private:
//    void ResizeMemoryPool(uint32 newCapacity)
//    {
//        uint32 newIndex = 0;
//        Vector<T> newObjectDataPool(newCapacity);
//        Vector<byte> newIsActive(newCapacity, 0x00);
//        for (uint32 oldIndex = 0; oldIndex < ObjectDataPool.size(); ++oldIndex)
//        {
//            if (IsActive[oldIndex])
//            {
//                newObjectDataPool[newIndex] = std::move(ObjectDataPool[oldIndex]);
//                newIsActive[newIndex] = 1;
//                newIndex++;
//            }
//        }
//        FreeIndices.clear();
//
//        for (uint32 x = newIndex; x < newCapacity; ++x)
//        {
//            FreeIndices.push_back(x);
//        }
//
//        VkDeviceSize newSize = sizeof(T) * newCapacity;
//        uint32 newBufferId = bufferSystem.VMACreateDynamicBuffer(nullptr, newSize, bufferSystem.FindVulkanBuffer(BufferId).BufferUsage);
//        bufferSystem.VMAUpdateDynamicBuffer(newBufferId, newObjectDataPool.data(), sizeof(T) * newIndex);
//
//        if (BufferId != UINT32_MAX)
//        {
//            bufferSystem.DestroyBuffer(bufferSystem.FindVulkanBuffer(BufferId));
//        }
//
//        BufferId = newBufferId;
//        ObjectDataPool = std::move(newObjectDataPool);
//        IsActive = std::move(newIsActive);
//        ActiveCount = newIndex;
//        IsDirty = true;
//    }
//
//public:
//    MemoryPool() = default;
//
//    ~MemoryPool()
//    {
//     /*   if (BufferId != UINT32_MAX)
//        {
//            bufferSystem.DestroyBuffer(bufferSystem.FindVulkanBuffer(BufferId));
//            BufferId = UINT32_MAX;
//        }*/
//    }
//
//    void CreateMemoryPool(uint32 initialCapacity, VkBufferUsageFlags usage)
//    {
//        ObjectDataPool.resize(initialCapacity);
//        IsActive.resize(initialCapacity, 0);
//        ActiveCount = 0;
//
//        VkDeviceSize size = sizeof(T) * initialCapacity;
//        BufferId = bufferSystem.VMACreateDynamicBuffer(nullptr, size, usage);
//
//        std::fill(ObjectDataPool.begin(), ObjectDataPool.end(), T{});
//    }
//
//    uint32 AllocateObject()
//    {
//        if (!FreeIndices.empty())
//        {
//            uint32 index = FreeIndices.back();
//            FreeIndices.pop_back();
//
//            ObjectDataPool[index] = T{};
//            IsActive[index] = 1;
//
//            if (index + 1 > ActiveCount)
//            {
//                ActiveCount = index + 1;
//            }
//
//            IsDirty = true;
//            return index;
//        }
//
//        if (ActiveCount == ObjectDataPool.size())
//        {
//            ResizeMemoryPool(ObjectDataPool.capacity() ? ObjectDataPool.capacity() * 2 : 1024);
//        }
//
//        uint32 index = ActiveCount++;
//        ObjectDataPool[index] = T{};
//        IsActive[index] = 0x01;
//        IsDirty = true;
//        return index;
//    }
//
//    void UpdateMemoryPool(uint32 descriptorBindingIndex, Vector<VulkanPipeline>& pipelineList)
//    {
//        if (ActiveCount == 0) return;
//        if (IsDirty)
//        {
//            MemoryPoolBufferHeader2 memoryPoolBufferHeader
//            {
//                .ObjectOffset = ActiveCount,
//                .ObjectCount = sizeof(MemoryPoolBufferHeader2),
//                .ObjectSize = sizeof(T),
//            };
//
//            Vector<byte> uploadData;
//            uploadData.resize(sizeof(MemoryPoolBufferHeader2) + ActiveCount * sizeof(T));
//            memcpy(uploadData.data(), &memoryPoolBufferHeader, sizeof(MemoryPoolBufferHeader2));
//            if (ActiveCount > 0)
//            {
//                memcpy(uploadData.data() + sizeof(MemoryPoolBufferHeader2), ObjectDataPool.data(), ActiveCount * sizeof(T));
//            }
//
//            bool bufferRecreated = false;
//            if (BufferId == UINT32_MAX)
//            {
//                BufferId = bufferSystem.VMACreateDynamicBuffer(nullptr, uploadData.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
//                bufferRecreated = true;
//            }
//            else if (bufferSystem.FindVulkanBuffer(BufferId).BufferSize < uploadData.size())
//            {
//                bufferSystem.DestroyBuffer(bufferSystem.FindVulkanBuffer(BufferId));
//                BufferId = bufferSystem.VMACreateDynamicBuffer(nullptr, uploadData.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
//                bufferRecreated = true;
//            }
//            bufferSystem.VMAUpdateDynamicBuffer(BufferId, uploadData.data(), uploadData.size());
//
//            Vector<VkDescriptorBufferInfo> bufferInfo = GetMemoryPoolBufferInfo();
//            for (auto& pipeline : pipelineList)
//            {
//                renderSystem.UpdateDescriptorSet(pipeline, bufferInfo, descriptorBindingIndex);
//            }
//            IsDirty = false;
//        }
//    }
//
//    void FreeDataSlot(uint32 index)
//    {
//        if (index >= IsActive.size() || !IsActive[index]) return;
//
//        FreeIndices.push_back(index);
//        IsActive[index] = 0;
//        IsDirty = true;
//    }
//
//    void DestroyMemoryPool()
//    {
//
//    }
//
//    T& Get(uint32 index)
//    {
//        if (index >= IsActive.size() || !IsActive[index])
//        {
//            static T invalid{};
//            return invalid;
//        }
//        return ObjectDataPool[index];
//    }
//
//    bool IsSlotActive(uint32 index) const
//    {
//        return index < IsActive.size() && IsActive[index];
//    }
//
//    template <typename Func>
//    void ForEachActive(Func&& func)
//    {
//        for (uint32_t x = 0; x < ObjectDataPool.size(); ++x)
//        {
//            if (IsActive[x])
//                func(x, ObjectDataPool[x]);
//        }
//    }
//
//    template <typename Func>
//    void ForEachActive(Func&& func) const
//    {
//        for (uint32_t x = 0; x < ObjectDataPool.size(); ++x)
//        {
//            if (IsActive[x])
//                func(x, ObjectDataPool[x]);
//        }
//    }
//
//    Vector<T> GetActiveObjects() const
//    {
//        Vector<T> active;
//        active.reserve(ActiveCount);
//
//        ForEachActive([&](const T& obj)
//            {
//                active.push_back(obj);
//            });
//
//        return active;
//    }
//
//    const Vector<VkDescriptorBufferInfo> GetMemoryPoolBufferInfo() const
//    {
//        return Vector<VkDescriptorBufferInfo>
//        {
//            VkDescriptorBufferInfo
//            {
//                .buffer = bufferSystem.FindVulkanBuffer(BufferId).Buffer,
//                .offset = 0,
//                .range = VK_WHOLE_SIZE
//            }
//        };
//    }
//};