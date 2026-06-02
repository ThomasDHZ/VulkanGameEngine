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
        [DllImport("kernel32.dll", SetLastError = true, CharSet = CharSet.Auto)] private static extern IntPtr LoadLibrary(string lpFileName);
        public static void SetSharedDllDirectory()
        {
            if (_sharedDirSet) return;

            string dllDir = Path.GetDirectoryName(GameEngineDLL)!;
            if (!Directory.Exists(dllDir))
            {
                throw new DirectoryNotFoundException($"DLL folder missing: {dllDir}");
            }

            IntPtr gameEngineDLLPtr = LoadLibrary(GameEngineDLL);
            if (gameEngineDLLPtr == IntPtr.Zero)
            {
                throw new System.ComponentModel.Win32Exception(Marshal.GetLastWin32Error());
            }
            _sharedDirSet = true;
        }

        public static void CallDLLFunc(Action action)
        {
            if (!_sharedDirSet)
            {
                SetSharedDllDirectory();
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
                SetSharedDllDirectory();
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
