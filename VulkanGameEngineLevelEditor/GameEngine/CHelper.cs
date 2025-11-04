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
    public static class CHelper
    {
        public static IntPtr[] VectorToConstCharPtrPtr(List<string> strings)
        {
            if (strings == null || strings.Count == 0)
            {
                Console.WriteLine("CHelper.VectorToConstCharPtrPtr: Input list is null or empty.");
                return null;
            }

            try
            {
                IntPtr[] result = new IntPtr[strings.Count];
                for (int i = 0; i < strings.Count; i++)
                {
                    if (strings[i] == null)
                    {
                        Console.WriteLine($"CHelper.VectorToConstCharPtrPtr: String at index {i} is null.");
                        return null; 
                    }
                    result[i] = Marshal.StringToHGlobalAnsi(strings[i]);
                    if (result[i] == IntPtr.Zero)
                    {
                        Console.WriteLine($"CHelper.VectorToConstCharPtrPtr: Failed to allocate memory for string at index {i}.");

                        for (int j = 0; j < i; j++)
                            Marshal.FreeHGlobal(result[j]);
                        return null;
                    }
                }
                return result;
            }
            catch (Exception ex)
            {
                Console.WriteLine($"CHelper.VectorToConstCharPtrPtr: Exception occurred: {ex.Message}");
                return null;
            }
        }

        public static void CHelper_DestroyConstCharPtrPtr(IntPtr[] ptrs)
        {
            if (ptrs == null) return;
            foreach (var ptr in ptrs)
            {
                if (ptr != IntPtr.Zero)
                    Marshal.FreeHGlobal(ptr);
            }
        }
    }
}
