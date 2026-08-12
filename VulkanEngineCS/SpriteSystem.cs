using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using VulkanEngineCoreCS;

namespace VulkanEngineCS
{
    public unsafe class SpriteSystem
    {
        public static void Update(float deltaTime)
        {
            DLLSystem.CallDLLFunc(() => SpriteSystem_Update(deltaTime));
        }
        [DllImport("VulkanEngineDLL.dll", CallingConvention = CallingConvention.Cdecl)] private static extern void SpriteSystem_Update(float deltaTime);
    }
}
