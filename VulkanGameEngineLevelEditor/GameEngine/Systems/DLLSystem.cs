using Newtonsoft.Json.Linq;
using System;
using System.IO;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Text;
using System.Windows.Forms;

namespace VulkanGameEngineLevelEditor.GameEngine.Systems
{
    public static class DLLSystem
    {
        private static bool _sharedDirSet = false;
        private static string ExeDirectory => Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location)!;
        private static string EngineRoot => Path.GetFullPath(Path.Combine(ExeDirectory, @"..\..\..\..\"));
        public static string GameEnginePath => Path.Combine(EngineRoot, @"C:\Users\dotha\Documents\GitHub\VulkanGameEngine\x64\Debug\VulkanEngineDLL.dll");
        public static string Game2DPath => Path.Combine(EngineRoot, @"C:\Users\dotha\Documents\GitHub\VulkanGameEngine\x64\Debug\ComponentDLL.dll");
        public const string GameEngineDLL = @"C:\Users\dotha\Documents\GitHub\VulkanGameEngine\x64\Debug\VulkanEngineDLL.dll";
        public const string Game2DDLL = @"C:\Users\dotha\Documents\GitHub\VulkanGameEngine\x64\Debug\ComponentDLL.dll";

        [DllImport("kernel32.dll", SetLastError = true, CharSet = CharSet.Auto)] private static extern IntPtr LoadLibrary(string lpFileName);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern void Debug_SetRootDirectory([MarshalAs(System.Runtime.InteropServices.UnmanagedType.LPStr)] string engineRoot);

        public static void SetSharedDllDirectory()
        {
            if (_sharedDirSet) return;

            string dllDir = Path.GetDirectoryName(GameEnginePath)!;
            if (!Directory.Exists(dllDir))
            {
                throw new DirectoryNotFoundException($"DLL folder missing: {dllDir}");
            }

            Debug_SetRootDirectory("..\\..\\..\\..\\Assets");
            IntPtr gameEngineDLLPtr = LoadLibrary(GameEnginePath);
            IntPtr game2DDLLPtr = LoadLibrary(Game2DDLL);
            if (gameEngineDLLPtr == IntPtr.Zero ||
                game2DDLLPtr == IntPtr.Zero)
            {
                throw new System.ComponentModel.Win32Exception(Marshal.GetLastWin32Error());
            }
            _sharedDirSet = true;
        }

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
                MessageBox.Show(ex.ToString());
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
                MessageBox.Show(ex.ToString());
                return default(TResult);
            }
        }

    }
}