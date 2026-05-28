using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace GameScriptLibraryDLL
{
    public static class DLLSystem
    {
        private static bool _sharedDirSet = false;
        public const string GameEngineDLL = @"C:\Users\DHZ\Documents\GitHub\VulkanGameEngine\x64\Debug\VulkanGameEngineLevelEditorInterlopDLL.dll";
        public static void CallDLLFunc(Action action)
        {
            if (!_sharedDirSet)
            {
                throw new InvalidOperationException("Call SetSharedDllDirectory() first!");
            }

            try
            {
                action();
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.ToString());
            }
        }

        public static TResult CallDLLFunc<TResult>(Func<TResult> func)
        {
            if (!_sharedDirSet)
            {
                throw new InvalidOperationException("Call SetSharedDllDirectory() first!");
            }

            try
            {
                TResult result = func();
                if (typeof(TResult) == typeof(IntPtr))
                {
                    IntPtr ptr = (IntPtr)(object)result;
                    if (ptr == IntPtr.Zero)
                    {
                        return default;
                    }
                    TResult copyResult = Marshal.PtrToStructure<TResult>(ptr);
                    MemorySystem.RemovePtrBuffer(ptr);
                    return copyResult;
                }
                return result;
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.ToString());
                return default(TResult);
            }
        }
    }
}
