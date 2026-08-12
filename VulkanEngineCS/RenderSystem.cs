using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using VulkanEngineCoreCS;

namespace VulkanEngineCS
{
    public unsafe class RenderSystem
    {
        public static Guid LoadRenderPass(string jsonPath)
        {
           return DLLSystem.CallDLLFunc(() => RenderSystem_LoadRenderPass(jsonPath));
        }

        public static void Update(void* windowHandle, float deltaTime)
        {
            DLLSystem.CallDLLFunc(() => RenderSystem_Update(windowHandle, deltaTime));
        }

        [DllImport("VulkanEngineDLL.dll", CallingConvention = CallingConvention.Cdecl)] private static extern Guid RenderSystem_LoadRenderPass([MarshalAs(UnmanagedType.LPStr)] string jsonPath);
        [DllImport("VulkanEngineDLL.dll", CallingConvention = CallingConvention.Cdecl)] private static extern void RenderSystem_Update(void* windowHandle, float deltaTime);
    }
}
