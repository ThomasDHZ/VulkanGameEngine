using GlmSharp;
using Microsoft.VisualBasic;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using Vulkan;
using VulkanGameEngineLevelEditor.GameEngine.Structs;
using VulkanGameEngineLevelEditor.GameEngine.Systems;

namespace VulkanGameEngineLevelEditor.GameEngine
{
    public unsafe static class CHelper
    {

        public static IntPtr[] VectorToConstCharPtrPtr(List<String> list)
        {
            IntPtr[] stringPointers = new IntPtr[list.Count];
            for (int x = 0; x < list.Count; x++)
            {
                stringPointers[x] = Marshal.StringToHGlobalAnsi(list[x]);
            }
            return stringPointers;
        }

        public static void CHelper_DestroyConstCharPtrPtr(IntPtr[] constCharPtrPtr)
        {
            foreach (var ptr in constCharPtrPtr)
            {
                Marshal.FreeHGlobal(ptr);
            }
        }
    }
}
