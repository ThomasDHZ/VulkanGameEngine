using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using VulkanEngineCoreCS;

namespace VulkanEngineCS
{
    public unsafe class MemoryPoolSystem
    {
        public static void StartUp()
        { 
            DLLSystem.CallDLLFunc(() => MemoryPoolSystem_StartUp());
        }

        public static void Update()
        {
            DLLSystem.CallDLLFunc(() => MemoryPoolSystem_Update());
        }

        [DllImport("VulkanEngineDLL.dll", CallingConvention = CallingConvention.Cdecl)] private static extern void MemoryPoolSystem_StartUp();
        [DllImport("VulkanEngineDLL.dll", CallingConvention = CallingConvention.Cdecl)] private static extern void MemoryPoolSystem_Update();
    }
}
