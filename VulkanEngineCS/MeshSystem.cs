using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using VulkanEngineCoreCS;

namespace VulkanEngineCS
{
    public unsafe class MeshSystem
    {
        public static void Update(float deltaTime)
        {
            DLLSystem.CallDLLFunc(() => MeshSystem_Update(deltaTime));
        }
        [DllImport("VulkanEngineInterop.dll", CallingConvention = CallingConvention.Cdecl)] private static extern void MeshSystem_Update(float deltaTime);
    }
}
