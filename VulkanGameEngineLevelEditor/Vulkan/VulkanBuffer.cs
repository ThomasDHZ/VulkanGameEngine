﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using VulkanGameEngineLevelEditor.GameEngineAPI;

namespace VulkanGameEngineLevelEditor.Vulkan
{
    public unsafe class VulkanBuffer<T> where T : unmanaged
    {
        protected VkDevice _device => RenderSystem.Device;
        protected VkPhysicalDevice _physicalDevice => RenderSystem.PhysicalDevice;
        protected VkCommandPool _commandPool => RenderSystem.CommandPool;
        protected VkQueue _graphicsQueue => RenderSystem.GraphicsQueue;

        public VkBuffer StagingBuffer;
        public VkDeviceMemory StagingBufferMemory;
        public VkDeviceMemory BufferMemory;
        public ulong BufferSize = 0;
        public VkBufferUsageFlagBits BufferUsage;
        public VkMemoryPropertyFlagBits BufferProperties;
        public ulong BufferDeviceAddress = 0;
        public IntPtr BufferData;
        public bool IsMapped = false;
        public bool UsingStagingBuffer = false;
        public VkBuffer Buffer;
        public VkDescriptorBufferInfo DescriptorBufferInfo;

        public VulkanBuffer()
        {
        }

        public VulkanBuffer(VkBuffer stagingBuffer, VkDeviceMemory stagingBufferMemory, VkDeviceMemory bufferMemory, VkDeviceSize bufferSize, VkBufferUsageFlagBits bufferUsage, VkMemoryPropertyFlagBits bufferProperties)
        {
            StagingBuffer = stagingBuffer;
            StagingBufferMemory = stagingBufferMemory;
            BufferMemory = bufferMemory;
            BufferSize = bufferSize;
            BufferUsage = bufferUsage;
            BufferProperties = bufferProperties;
        }

        public VulkanBuffer(void* bufferData, uint bufferElementCount, VkBufferUsageFlagBits usage, VkMemoryPropertyFlagBits properties, bool usingStagingBuffer)
        {
            BufferSize = (ulong)sizeof(T) * bufferElementCount;
            BufferProperties = properties;
            UsingStagingBuffer = usingStagingBuffer;
            BufferUsage = usage;

            if (UsingStagingBuffer)
            {
                CreateStagingBuffer(bufferData);
            }
            else
            {
                CreateBuffer(bufferData);
            }
        }

        public VulkanBuffer(IntPtr bufferData, uint bufferElementCount, VkBufferUsageFlagBits usage, VkMemoryPropertyFlagBits properties, bool usingStagingBuffer)
        {
            BufferSize = (ulong)sizeof(T) * bufferElementCount;
            BufferProperties = properties;
            UsingStagingBuffer = usingStagingBuffer;
            BufferUsage = usage;

            if (UsingStagingBuffer)
            {
                CreateStagingBuffer((void*)bufferData);
            }
            else
            {
                CreateBuffer((void*)bufferData);
            }
        }

        public VulkanBuffer(T bufferData, VkBufferUsageFlagBits usage, VkMemoryPropertyFlagBits properties, bool usingStagingBuffer)
        {
            BufferSize = (ulong)sizeof(T);
            BufferProperties = properties;
            UsingStagingBuffer = usingStagingBuffer;
            BufferUsage = usage;

            void* bufferDataPtr = &bufferData;

            if (UsingStagingBuffer)
            {
                CreateStagingBuffer(bufferDataPtr);
            }
            else
            {
                CreateBuffer(bufferDataPtr);
            }
        }

        public VulkanBuffer(List<T> bufferDataList, VkBufferUsageFlagBits usage, VkMemoryPropertyFlagBits properties, bool usingStagingBuffer)
        {
            var arrayList = bufferDataList.ToArray();

            BufferSize = (ulong)(sizeof(T) * bufferDataList.UCount());
            BufferProperties = properties;
            UsingStagingBuffer = usingStagingBuffer;
            BufferUsage = usage;

            GCHandle handle = GCHandle.Alloc(arrayList, GCHandleType.Pinned);
            void* bufferDataPtr = (void*)handle.AddrOfPinnedObject();

            if (UsingStagingBuffer)
            {
                CreateStagingBuffer(bufferDataPtr);
            }
            else
            {
                CreateBuffer(bufferDataPtr);
            }

            handle.Free();
        }

        public VulkanBuffer(List<T> bufferDataList, uint reserveCount, VkBufferUsageFlagBits usage, VkMemoryPropertyFlagBits properties, bool usingStagingBuffer)
        {
            BufferSize = (ulong)(sizeof(T) * (reserveCount + bufferDataList.Count));
            BufferProperties = properties;
            UsingStagingBuffer = usingStagingBuffer;
            BufferUsage = usage;

            GCHandle handle = GCHandle.Alloc(bufferDataList, GCHandleType.Pinned);
            void* bufferDataPtr = (void*)handle.AddrOfPinnedObject();

            if (UsingStagingBuffer)
            {
                CreateStagingBuffer(bufferDataPtr);
            }
            else
            {
                CreateBuffer(bufferDataPtr);
            }

            handle.Free();
        }

        private VkResult CreateBuffer(void* bufferData)
        {
            VkResult result = GameEngineImport.DLL_Buffer_CreateBuffer(_device, _physicalDevice, out VkBuffer buffer, out VkDeviceMemory bufferMemory, bufferData, BufferSize, BufferProperties, BufferUsage);
            Buffer = buffer;
            BufferMemory = bufferMemory;

            return result;
        }

        private VkResult CreateStagingBuffer(void* bufferData)
        {
            VkResult result = GameEngineImport.DLL_Buffer_CreateStagingBuffer(_device, _physicalDevice, _commandPool, _graphicsQueue, out VkBuffer stagingBuffer, out VkDeviceMemory stagingBufferMemory, out VkBuffer buffer, out VkDeviceMemory bufferMemory, bufferData, BufferSize, BufferUsage, BufferProperties);
            StagingBuffer = stagingBuffer;
            Buffer = buffer;
            BufferMemory = bufferMemory;
            StagingBufferMemory = stagingBufferMemory;
            return result;
        }

        private VkResult UpdateBufferSize(VkBuffer buffer, VkDeviceMemory bufferMemory, VkDeviceSize newBufferSize)
        {
            GCHandle bufferSizeHandle = GCHandle.Alloc(BufferSize, GCHandleType.Pinned);
            VkResult result = GameEngineImport.DLL_Buffer_UpdateBufferSize(_device, _physicalDevice, buffer, &bufferMemory, (void*)BufferData, (ulong*)bufferSizeHandle.AddrOfPinnedObject(), newBufferSize, BufferUsage, BufferProperties);
            DestroyBuffer();
            return result;
        }

        public static VkResult CopyBuffer(VkBuffer* srcBuffer, VkBuffer* dstBuffer, VkDeviceSize size)
        {
            VkDevice _device = RenderSystem.Device;
            VkCommandPool _commandPool = RenderSystem.CommandPool;
            VkQueue _graphicsQueue = RenderSystem.GraphicsQueue;
            return GameEngineImport.DLL_Buffer_CopyBuffer(_device, _commandPool, _graphicsQueue, srcBuffer, dstBuffer, size);
        }

        public void UpdateBufferMemory(IntPtr bufferData)
        {
            if (UsingStagingBuffer)
            {
                GameEngineImport.DLL_Buffer_UpdateStagingBufferData(_device, _commandPool, _graphicsQueue, StagingBuffer, Buffer, out VkBuffer stagingBufferMemory, out VkBuffer bufferMemory, (void*)bufferData, BufferSize);
                StagingBuffer = stagingBufferMemory;
                Buffer = bufferMemory;
            }
            else
            {
                var bufferMemory = BufferMemory;
                GameEngineImport.DLL_Buffer_UpdateBufferData(_device, &bufferMemory, (void*)bufferData, BufferSize);
            }
        }

        public void UpdateBufferMemory(T bufferData)
        {

            GCHandle bufferDataHandle = GCHandle.Alloc(bufferData, GCHandleType.Pinned);
            if (UsingStagingBuffer)
            {
                GameEngineImport.DLL_Buffer_UpdateStagingBufferData(_device, _commandPool, _graphicsQueue, StagingBuffer, Buffer, out VkBuffer stagingBufferMemory, out VkBuffer bufferMemory, (void*)bufferDataHandle.AddrOfPinnedObject(), BufferSize);
                StagingBuffer = stagingBufferMemory;
                Buffer = bufferMemory;
            }
            else
            {
                var bufferMemory = BufferMemory;
                GameEngineImport.DLL_Buffer_UpdateBufferData(_device, &bufferMemory, (void*)bufferDataHandle.AddrOfPinnedObject(), BufferSize);
            }
            bufferDataHandle.Free();
        }

        public void UpdateBufferMemory(List<T> bufferData)
        {
            VkDeviceSize newBufferSize = (ulong)(sizeof(T) * bufferData.Count());
            GCHandle bufferDataHandle = GCHandle.Alloc(bufferData, GCHandleType.Pinned);
            if (UsingStagingBuffer)
            {
                if (BufferSize != newBufferSize)
                {
                    if (UpdateBufferSize(StagingBuffer, StagingBufferMemory, newBufferSize) != VkResult.VK_SUCCESS)
                    {
                        bufferDataHandle.Free();
                        throw new Exception("Failed to update staging buffer size.");
                    }
                }

                GameEngineImport.DLL_Buffer_UpdateStagingBufferData(_device, _commandPool, _graphicsQueue, StagingBuffer, Buffer, out VkBuffer stagingBufferMemory, out VkBuffer bufferMemory, (nint*)bufferDataHandle.AddrOfPinnedObject(), BufferSize);
                StagingBuffer = stagingBufferMemory;
                Buffer = bufferMemory;
            }
            else
            {
                if (BufferSize != newBufferSize)
                {
                    if (UpdateBufferSize(Buffer, BufferMemory, newBufferSize) != VkResult.VK_SUCCESS)
                    {
                        bufferDataHandle.Free();
                        throw new Exception("Failed to update buffer size.");
                    }
                }

                VkResult result = GameEngineImport.DLL_Buffer_UpdateBufferMemory(_device, BufferMemory, (nint*)bufferDataHandle.AddrOfPinnedObject(), newBufferSize);
                if (result != VkResult.VK_SUCCESS)
                {
                    bufferDataHandle.Free();
                    throw new Exception("Failed to update buffer memory.");
                }
            }
            bufferDataHandle.Free();
        }

        public void UpdateBufferMemory(void* bufferData, VkDeviceSize totalBufferSize)
        {
            VkDeviceSize newBufferSize = totalBufferSize;
            if (UsingStagingBuffer)
            {
                if (BufferSize != newBufferSize)
                {
                    if (UpdateBufferSize(StagingBuffer, StagingBufferMemory, newBufferSize) != VkResult.VK_SUCCESS)
                    {
                        throw new Exception("Failed to update staging buffer size.");
                    }
                }

                GameEngineImport.DLL_Buffer_UpdateStagingBufferData(_device, _commandPool, _graphicsQueue, StagingBuffer, Buffer, out VkBuffer stagingBufferMemory, out VkBuffer bufferMemory, bufferData, BufferSize);
                StagingBuffer = stagingBufferMemory;
                Buffer = bufferMemory;
            }
            else
            {
                if (BufferSize != newBufferSize)
                {
                    if (UpdateBufferSize(Buffer, BufferMemory, newBufferSize) != VkResult.VK_SUCCESS)
                    {
                        throw new Exception("Failed to update buffer size.");
                    }
                }

                VkResult result = GameEngineImport.DLL_Buffer_UpdateBufferMemory(_device, BufferMemory, bufferData, newBufferSize);
                if (result != VkResult.VK_SUCCESS)
                {
                    throw new Exception("Failed to update buffer memory.");
                }
            }
        }

        public List<T> CheckBufferContents()
        {
            List<T> DataList = new List<T>();
            size_t dataListSize = (nint)(BufferSize / (ulong)sizeof(T));

            var isMapped = IsMapped;
            void* data = GameEngineImport.DLL_Buffer_MapBufferMemory(_device, BufferMemory, BufferSize, &isMapped);
            if (data == null)
            {
                throw new Exception("Failed to map buffer memory.");
            }

            char* newPtr = (char*)data;
            for (size_t x = 0; x < dataListSize; ++x)
            {
                DataList.Add(*((T*)newPtr));
                newPtr += sizeof(T);
            }
            GameEngineImport.DLL_Buffer_UnmapBufferMemory(_device, BufferMemory, &isMapped);
            IsMapped = isMapped;

            return DataList;
        }

        public void DestroyBuffer()
        {
            GCHandle bufferDataHandle = GCHandle.Alloc(BufferData, GCHandleType.Pinned);
            GCHandle stagingBufferHandle = GCHandle.Alloc(StagingBuffer, GCHandleType.Pinned);
            GCHandle bufferHandle = GCHandle.Alloc(Buffer, GCHandleType.Pinned);
            GCHandle stagingBufferMemoryHandle = GCHandle.Alloc(StagingBuffer, GCHandleType.Pinned);
            GCHandle bufferMemoryHandle = GCHandle.Alloc(Buffer, GCHandleType.Pinned);

            fixed (ulong* sizePtr = &BufferSize)
            fixed (VkBufferUsageFlagBits* usage = &BufferUsage)
            fixed (VkMemoryPropertyFlagBits* properties = &BufferProperties)
            {
                GameEngineImport.DLL_Buffer_DestroyBuffer(_device, (nint*)bufferHandle.AddrOfPinnedObject(),
                                                                   (nint*)stagingBufferHandle.AddrOfPinnedObject(),
                                                                   (nint*)bufferMemoryHandle.AddrOfPinnedObject(),
                                                                   (nint*)stagingBufferMemoryHandle.AddrOfPinnedObject(),
                                                                   (nint*)bufferDataHandle.AddrOfPinnedObject(), sizePtr, usage, properties);
            }

            bufferDataHandle.Free();
            stagingBufferMemoryHandle.Free();
            bufferMemoryHandle.Free();
            bufferDataHandle.Free();
            stagingBufferHandle.Free();
        }
    }
}