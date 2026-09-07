using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using VulkanEngineCoreCS;

namespace VulkanEngineCS
{
    public unsafe class TextureSystem
    {
        public static void Destroy()
        {
            DLLSystem.CallDLLFunc(() => TextureSystem_Destroy());
        }
        [DllImport("VulkanEngineInterop.dll", CallingConvention = CallingConvention.Cdecl)] private static extern void TextureSystem_Destroy();
    }
}
