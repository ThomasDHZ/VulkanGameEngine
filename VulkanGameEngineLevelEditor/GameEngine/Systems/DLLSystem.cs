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
        private static string EngineRoot => Path.GetFullPath(Path.Combine(ExeDirectory, @"..\..\..\..\VulkanGameEngine"));
        private static string GameEnginePath => Path.Combine(EngineRoot, @"x64\Debug\VulkanEngineDLL.dll");
        private static string Game2DPath => Path.Combine(EngineRoot, @"x64\Debug\ComponentDLL.dll");
        public const string GameEngineDLL = @"..\..\..\..\x64\Debug\VulkanEngineDLL.dll";
        public const string Game2DDLL = @"..\..\..\..\x64\Debug\ComponentDLL.dll";
        public static void SetSharedDllDirectory()
        {
            if (_sharedDirSet) return;

            string dllDir = Path.GetDirectoryName(GameEnginePath)!;
            if (!Directory.Exists(dllDir))
            {
                throw new DirectoryNotFoundException($"DLL folder missing: {dllDir}");
            }

            Directory.SetCurrentDirectory(dllDir);
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
                return func();

            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.ToString());
                MessageBox.Show(ex.ToString());
                return default(TResult);
            }
        }

        [DllImport("kernel32.dll", SetLastError = true, CharSet = CharSet.Auto)] private static extern IntPtr LoadLibrary(string lpFileName);

        [DllImport("kernel32.dll", CharSet = CharSet.Auto, SetLastError = true)] private static extern IntPtr GetModuleHandle(string lpModuleName);

        [DllImport("kernel32.dll", CharSet = CharSet.Auto, SetLastError = true)] [return: MarshalAs(UnmanagedType.Bool)] private static extern bool GetModuleFileName(IntPtr hModule, StringBuilder lpFilename, uint nSize);
    }
}