using System;
using System.Configuration;

namespace VulkanGameEngineLevelEditor.LevelEditor
{
    public static class ConstConfig
    {
        public static string BaseDirectoryPath => $@"{AppDomain.CurrentDomain.BaseDirectory}..\..\..\..\";
        public static string ShaderCompilerPath => @"C:\VulkanSDK\1.4.313.0\Bin\glslc.exe";
    }
}