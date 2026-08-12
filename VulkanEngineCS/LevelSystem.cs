using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using VulkanEngineCoreCS;

namespace VulkanEngineCS
{
    public unsafe class LevelSystem
    {
        public static void LoadLevel([MarshalAs(UnmanagedType.LPStr)] string levelPath)
        {
            DLLSystem.CallDLLFunc(() => LevelSystem_LoadLevel(levelPath));
        }

        public static void Update(float deltaTime)
        {
            DLLSystem.CallDLLFunc(() => LevelSystem_Update(deltaTime));
        }

        [DllImport("VulkanEngineDLL.dll", CallingConvention = CallingConvention.Cdecl)] private static extern void LevelSystem_LoadLevel([MarshalAs(UnmanagedType.LPStr)] string levelPath);
        [DllImport("VulkanEngineDLL.dll", CallingConvention = CallingConvention.Cdecl)] private static extern void LevelSystem_Update(float deltaTime);
    }
}
