using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using VulkanEngineCoreCS;

namespace VulkanEngineCS
{
    public static class InputSystem
    {
        public static void Update(float deltaTime)
        {
            DLLSystem.CallDLLFunc(() => InputSystem_LoadRenderPass(deltaTime));
        }

        [DllImport("VulkanEngineDLL.dll", CallingConvention = CallingConvention.Cdecl)] private static extern void InputSystem_LoadRenderPass(float deltaTime);
    }
}
