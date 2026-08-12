using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using VulkanEngineCoreCS;

namespace VulkanEngineCS
{
    public unsafe class CSharpScriptSystem
    {
        public static bool Initialize()
        {
            return DLLSystem.CallDLLFunc(() => CSharpScriptSystem_Initialize());
        }
        [DllImport("VulkanEngineDLL.dll", CallingConvention = CallingConvention.Cdecl)] private static extern bool CSharpScriptSystem_Initialize();
    }
}
