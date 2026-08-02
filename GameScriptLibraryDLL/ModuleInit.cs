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
        public const String VulkanEngineDLL = "VulkanEngineDLL.dll";
        public const String VulkanEngineCoreDLL = "VulkanEngineCore.dll";

        [ModuleInitializer]
        internal static void Initialize()
        {
            DLLSystem.Initialize();
        }
    }
}
