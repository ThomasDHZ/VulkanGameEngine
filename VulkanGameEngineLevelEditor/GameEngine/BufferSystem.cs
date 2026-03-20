using Silk.NET.Vulkan;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using VulkanCS;

namespace VulkanGameEngineLevelEditor.GameEngine
{
    public unsafe class BufferSystem
    {
        public static uint CreateStaticVulkanBuffer(void* srcData, VkDeviceSize size, VkBufferUsageFlagBits shaderUsageFlags, VkDeviceSize offset)
        {
            return DLLSystem.CallDLLFunc(() => BufferSystem_CreateStaticVulkanBuffer(srcData, size, shaderUsageFlags, offset));
        }

        public static uint CreateDynamicBuffer(void* srcData, VkDeviceSize size, VkBufferUsageFlagBits usageFlags)
        {
            return DLLSystem.CallDLLFunc(() => BufferSystem_CreateDynamicBuffer(srcData, size, usageFlags));
        }
        public static void UpdateDynamicBuffer(uint bufferId, void* data, VkDeviceSize size, VkDeviceSize offset = 0)
        {
            DLLSystem.CallDLLFunc(() => BufferSystem_UpdateDynamicBuffer(bufferId, data, size, offset));
        }

        public static void CopyBuffer(VkBuffer* srcBuffer, VkBuffer* dstBuffer, VkDeviceSize size, VkBufferUsageFlagBits shaderUsageFlags, VkDeviceSize offset = 0)
        {
            DLLSystem.CallDLLFunc(() => BufferSystem_CopyBuffer(srcBuffer, dstBuffer, size, shaderUsageFlags, offset));
        }
        public static void DestroyBuffer(uint bufferId)
        {
            DLLSystem.CallDLLFunc(() => BufferSystem_DestroyBuffer(bufferId));
        }

        public static void DestroyAllBuffers()
        {
            DLLSystem.CallDLLFunc(() => BufferSystem_DestroyAllBuffers());
        }

        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern uint BufferSystem_CreateStaticVulkanBuffer(void* srcData, VkDeviceSize size, VkBufferUsageFlagBits shaderUsageFlags, VkDeviceSize offset = 0);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern uint BufferSystem_CreateDynamicBuffer(void* srcData, VkDeviceSize size, VkBufferUsageFlagBits usageFlags);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern void BufferSystem_UpdateDynamicBuffer(uint bufferId, void* data, VkDeviceSize size, VkDeviceSize offset = 0);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern void BufferSystem_CopyBuffer(VkBuffer* srcBuffer, VkBuffer* dstBuffer, VkDeviceSize size, VkBufferUsageFlagBits shaderUsageFlags, VkDeviceSize offset = 0);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern void BufferSystem_DestroyBuffer(uint bufferId);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern void BufferSystem_DestroyAllBuffers();
    }
}
