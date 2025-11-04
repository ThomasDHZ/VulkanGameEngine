using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using Vulkan;

namespace VulkanGameEngineLevelEditor.GameEngine.Systems
{
    public static class GPUSystem
    {
        public static bool CheckRayTracingCompatiblity(VkPhysicalDevice gpuDevice)
        {
            return DLLSystem.CallDLLFunc(() => GPUSystem_CheckRayTracingCompatiblity(gpuDevice));
        }

        public static VkSampleCountFlagBits GetMaxUsableSampleCount(VkPhysicalDevice GPUDevice)
        {
            return DLLSystem.CallDLLFunc(() => GPUSystem_GetMaxUsableSampleCount(GPUDevice));
        }

        public static void StartUp()
        {
            DLLSystem.CallDLLFunc(() => GPUSystem_StartUp());
        }

        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern bool GPUSystem_CheckRayTracingCompatiblity(VkPhysicalDevice gpuDevice);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern VkSampleCountFlagBits GPUSystem_GetMaxUsableSampleCount(VkPhysicalDevice GPUDevice);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern void GPUSystem_StartUp();
    }
}


