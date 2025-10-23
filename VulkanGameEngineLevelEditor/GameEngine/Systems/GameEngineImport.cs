using GlmSharp;
using Silk.NET.SDL;
using Silk.NET.Vulkan;
using StbImageSharp;
using System.Numerics;
using System;
using System.Runtime.InteropServices;
using VulkanGameEngineLevelEditor.GameEngineAPI;
using VulkanGameEngineLevelEditor.Models;
using System.Collections.Generic;
using System.Reflection;

namespace VulkanGameEngineLevelEditor.GameEngine.Systems
{
    public unsafe class GameEngineImport
    {
        public const string DLLPath = "C:\\Users\\dotha\\Documents\\GitHub\\VulkanGameEngine\\x64\\Debug\\VulkanEngineDLL.dll";
        public const string Game2DPath = "C:\\Users\\dotha\\Documents\\GitHub\\VulkanGameEngine\\x64\\Debug\\ComponentDLL.dll";
    }
}
