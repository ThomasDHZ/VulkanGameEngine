using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;
using VulkanEngineCoreCS;

namespace GameScriptLibraryDLL
{
    public static class Module
    {
        public const String VulkanEngineDLL     = "C:\\Users\\DHZ\\Documents\\GitHub\\VulkanGameEngine\\x64\\Debug\\VulkanEngineDLL.dll";
        public const String VulkanEngineCoreDLL = "C:\\Users\\DHZ\\Documents\\GitHub\\VulkanEngineCore\\x64\\Debug\\VulkanEngineCore.dll";

        [ModuleInitializer]
        internal static void Initialize()
        {
            DLLSystem.Initialize(VulkanEngineDLL);
            DLLSystem.Initialize(VulkanEngineCoreDLL);
        }
    }
}
