using CSScripting;
using GlmSharp;
using Newtonsoft.Json;
using Silk.NET.SDL;
using Silk.NET.Vulkan;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Numerics;
using System.Runtime.InteropServices;
using System.Security.Policy;
using System.Text;
using System.Threading.Tasks;
using VulkanGameEngineLevelEditor.GameEngine.Structs;
using VulkanGameEngineLevelEditor.GameEngineAPI;
using VulkanGameEngineLevelEditor.LevelEditor;
using VulkanGameEngineLevelEditor.LevelEditor.EditorEnhancements;
using VulkanGameEngineLevelEditor.Models;

namespace VulkanGameEngineLevelEditor.GameEngine
{
    public unsafe static class LightSystem
    {
        public static void LoadSceneLights(string sceneLightJson)
        {
            DLLSystem.CallDLLFunc(() => LightSystem_LoadSceneLights(sceneLightJson));
        }
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern void LightSystem_LoadSceneLights([MarshalAs(System.Runtime.InteropServices.UnmanagedType.LPStr)] string sceneLightJson);
    }
}
