using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using VulkanEngineCoreCS;

namespace VulkanEngineCS
{
    public static class CollisionSystem
    {
        public static void Update()
        {
            DLLSystem.CallDLLFunc(() => CollisionSystem_Update());
        }
        [DllImport("VulkanEngineInterop.dll", CallingConvention = CallingConvention.Cdecl)] private static extern void CollisionSystem_Update();
    }
}
