using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using VulkanGameEngineLevelEditor.GameEngine.Structs;

namespace VulkanGameEngineLevelEditor.GameEngine.Systems
{
    public unsafe static class MemorySystem
    {
        private static readonly object lockObject = new object();
        public static Dictionary<IntPtr, MemoryLeakPtr> PtrAddressMap = new Dictionary<IntPtr, MemoryLeakPtr>(); 
        public static T* AddPtrBuffer<T>(size_t elementCount, string notes = "", [CallerFilePath] string file = "", [CallerLineNumber] int line = 0, [CallerMemberName] string member = "") where T : unmanaged
        {
            lock (lockObject)
            {
                MemoryLeakPtr memoryLeakPtr = MemoryLeakPtr_NewPtr(sizeof(T), elementCount, file, line, member, notes);
                PtrAddressMap[(IntPtr)memoryLeakPtr.PtrAddress] = memoryLeakPtr;
                return (T*)memoryLeakPtr.PtrAddress;
            }
        }

        public static T* AddPtrBuffer<T>(T elementData, string notes = "", [CallerFilePath] string file = "", [CallerLineNumber] int line = 0, [CallerMemberName] string member = "") where T : unmanaged
        {
            lock (lockObject)
            {
                MemoryLeakPtr memoryLeakPtr = MemoryLeakPtr_NewPtr(sizeof(T), 1, file, line, member, notes);
                PtrAddressMap[(IntPtr)memoryLeakPtr.PtrAddress] = memoryLeakPtr;
                return (T*)memoryLeakPtr.PtrAddress;
            }
        }

        public static unsafe T* AddPtrBuffer<T>(T* elementData, size_t elementCount, string notes = "", [CallerFilePath] string file = "", [CallerLineNumber] int line = 0, [CallerMemberName] string member = "") where T : unmanaged
        {
            lock (lockObject)
            {
                var memoryLeakPtr = MemoryLeakPtr_NewPtr(sizeof(T), elementCount, file, line, member, notes);
                if (memoryLeakPtr.PtrAddress == null || 
                   (IntPtr)memoryLeakPtr.PtrAddress == IntPtr.Zero)
                {
                    Console.Error.WriteLine($"Failed to allocate memory for {notes} at {file}:{line}");
                    return null;
                }

                T* buffer = (T*)memoryLeakPtr.PtrAddress;
                for (int x = 0; x < elementCount; x++)
                {
                    buffer[x] = elementData[x];
                }
                PtrAddressMap[(IntPtr)memoryLeakPtr.PtrAddress] = memoryLeakPtr;
                return buffer;
            }
        }

        public static void RemovePtrBuffer<T>(T* ptr)
        {
            lock (lockObject)
            {
                IntPtr voidPtr = (IntPtr)ptr;
                if (PtrAddressMap.ContainsKey(voidPtr))
                {
                    IntPtr memoryLeakPtr = voidPtr;
                    MemoryLeakPtr_DeletePtr(memoryLeakPtr);
                    PtrAddressMap.Remove(memoryLeakPtr);
                    ptr = null;
                }
                else
                {
                    Console.WriteLine(@$"Warning: Attempted to remove unregistered pointer: {ptr->ToString()}\n");
                }
            }
        }

        public static void RemovePtrBuffer(IntPtr ptr)
        {
            lock (lockObject)
            {
                if (PtrAddressMap.ContainsKey(ptr))
                {
                    MemoryLeakPtr_DeletePtr(ptr);
                    PtrAddressMap.Remove(ptr);
                    ptr = IntPtr.Zero;
                }
                else
                {
                    Console.WriteLine(@$"Warning: Attempted to remove unregistered pointer: {ptr.ToString()}\n");
                }
            }
        }

        public static void ReportLeaks()
        {
            lock (lockObject)
            {
                if (PtrAddressMap.Any())
                {
                    Console.WriteLine("Memory leaks detected in DLL:\n");
                    foreach (var ptr in PtrAddressMap)
                    {
                        var value = ptr.Value;
                        MemoryLeakPtr_DanglingPtrMessage(&value);
                    }
                }
            }
        }

        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] public static extern MemoryLeakPtr MemoryLeakPtr_NewPtr(size_t memorySize, size_t elementCount, [MarshalAs(UnmanagedType.LPStr)] string file, int line, [MarshalAs(UnmanagedType.LPStr)] string func, [MarshalAs(UnmanagedType.LPStr)] string notes);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] public static extern void MemoryLeakPtr_DeletePtr(IntPtr ptr);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] public static extern void MemoryLeakPtr_DanglingPtrMessage(MemoryLeakPtr* ptr);
    }
}
