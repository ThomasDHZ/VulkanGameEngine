using CSScripting;
using GlmSharp;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using Silk.NET.Core.Native;
using Silk.NET.SDL;
using Silk.NET.Vulkan;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Numerics;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using Vulkan;
using VulkanGameEngineLevelEditor.GameEngine.Structs;
using VulkanGameEngineLevelEditor.GameEngineAPI;
using VulkanGameEngineLevelEditor.Models;

namespace VulkanGameEngineLevelEditor.GameEngine.Systems
{
    public static unsafe class MaterialSystem
    {
        public static Guid CreateMaterial(string materialPath)
        {
            return DLLSystem.CallDLLFunc(() => MaterialSystem_CreateMaterial(materialPath));
        }

        public static void Update(float deltaTime)
        {
            DLLSystem.CallDLLFunc(() => MaterialSystem_Update(deltaTime));
        }

        public static bool MaterialMapExists(MaterialGuid materialGuid)
        {
            return DLLSystem.CallDLLFunc(() => MaterialSystem_MaterialMapExists(materialGuid));
        }

        public static Material FindMaterial(MaterialGuid materialGuid)
        {
            return DLLSystem.CallDLLFunc(() => MaterialSystem_FindMaterial(materialGuid));
        }

        public static void Destroy(MaterialGuid materialGuid)
        {
            DLLSystem.CallDLLFunc(() => MaterialSystem_Destroy(materialGuid));
        }

        public static void DestroyAllMaterials()
        {
            DLLSystem.CallDLLFunc(() => MaterialSystem_DestroyAllMaterials());
        }

        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern Guid MaterialSystem_CreateMaterial([MarshalAs(System.Runtime.InteropServices.UnmanagedType.LPStr)] string materialPath);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern void MaterialSystem_Update(float deltaTime);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern bool MaterialSystem_MaterialMapExists(MaterialGuid materialGuid);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern Material MaterialSystem_FindMaterial(MaterialGuid materialGuid);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern void MaterialSystem_Destroy(MaterialGuid materialGuid);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern void MaterialSystem_DestroyAllMaterials();
    }
}