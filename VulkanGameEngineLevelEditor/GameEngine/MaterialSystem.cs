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
using System.Reflection;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using Vulkan;
using VulkanGameEngineLevelEditor.GameEngine.Structs;
using VulkanGameEngineLevelEditor.GameEngineAPI;
using VulkanGameEngineLevelEditor.Models;

namespace VulkanGameEngineLevelEditor.GameEngine
{
    public struct Material
    {
        public Guid MaterialGuid { get; set; } = new Guid();
        public Guid AlbedoDataId { get; set; } = new Guid();
        public Guid NormalDataId { get; set; } = new Guid();
        public Guid PackedMRODataId { get; set; } = new Guid();
        public Guid PackedSheenSSSDataId { get; set; } = new Guid();
        public Guid UnusedDataId { get; set; } = new Guid();
        public Guid EmissionDataId { get; set; } = new Guid();
        public Material()
        {
        }
    };

    public static unsafe class MaterialSystem
    {
        public static Guid LoadMaterial(string materialPath)
        {
            return DLLSystem.CallDLLFunc(() => MaterialSystem_LoadMaterial(materialPath));
        }

        public static bool MaterialExists(MaterialGuid materialGuid)
        {
            return DLLSystem.CallDLLFunc(() => MaterialSystem_MaterialExists(materialGuid));
        }

        public static Material* FindMaterial(MaterialGuid materialGuid)
        {
            try
            {
                return MaterialSystem_FindMaterial(materialGuid);
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.ToString());
                MessageBox.Show(ex.ToString());
                return null;
            }
        }

        public static void Destroy(MaterialGuid materialGuid)
        {
            DLLSystem.CallDLLFunc(() => MaterialSystem_Destroy(materialGuid));
        }

        public static void DestroyAllMaterials()
        {
            DLLSystem.CallDLLFunc(() => MaterialSystem_DestroyAllMaterials());
        }

        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern Guid MaterialSystem_LoadMaterial([MarshalAs(System.Runtime.InteropServices.UnmanagedType.LPStr)] string materialPath);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern bool MaterialSystem_MaterialExists(Guid materialGuid);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern Material* MaterialSystem_FindMaterial(Guid materialGuid);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern uint MaterialSystem_FindMaterialPoolIndex(Guid materialGuid);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern void MaterialSystem_Destroy(Guid materialGuid);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern void MaterialSystem_DestroyAllMaterials();
    }
}