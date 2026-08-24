using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using VulkanEngineCoreCS;

namespace VulkanEngineCS
{
    public static unsafe class MemorySystem
    {
        /// <summary>
        /// Deletes a pointer that was allocated on the C++ side via the memory system.
        /// </summary>
        /// <typeparam name="T">The type the pointer was originally created for.</typeparam>
        /// <param name="obj">The managed object / pointer handle to free.</param>
        /// <remarks>
        /// Only call this on pointers that were returned by AddPtrBuffer / AddStringPtrBuffer etc.
        /// Calling it on a normal C# object will cause a crash.
        /// </remarks>
        public static void DeletePtr<T>(T obj) where T : class
        {
            try
            {
                MemorySystem_DeletePtr(*(void**)&obj);
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.ToString());
            }
        }

        public static void DeletePtr<T>(T* obj) where T : unmanaged
        {
            if (obj == null) return;
            try
            {
                MemorySystem_DeletePtr((void*)obj);
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.ToString());
            }
        }
        [DllImport("VulkanEngineInterop.dll", CallingConvention = CallingConvention.StdCall)] private static extern void MemorySystem_DeletePtr(void* ptr);
    }
}
