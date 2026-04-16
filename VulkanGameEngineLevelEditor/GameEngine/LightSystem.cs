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
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Security.Policy;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using VulkanGameEngineLevelEditor.GameEngine.Structs;
using VulkanGameEngineLevelEditor.GameEngineAPI;
using VulkanGameEngineLevelEditor.LevelEditor;
using VulkanGameEngineLevelEditor.LevelEditor.EditorEnhancements;
using VulkanGameEngineLevelEditor.Models;

namespace VulkanGameEngineLevelEditor.GameEngine
{
    public enum LightTypeEnum
    {
        kDirectionalLight,
        kPointLight
    };

    public unsafe static class LightSystem
    {
        public static uint LoadLight(string lightJson)
        {
            try
            {
                return LightSystem_LoadLight(lightJson);
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.ToString());
                MessageBox.Show(ex.ToString());
                return uint.MaxValue;
            }
        }

        public static DirectionalLight* LoadDirectionalLight()
        {
            try
            {
                uint lightId = LightSystem_AllocateLight(LightTypeEnum.kDirectionalLight);
                return MemoryPoolSystem.UpdateDirectionalLight(lightId);
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.ToString());
                MessageBox.Show(ex.ToString());
                return null;
            }
        }

        public static PointLight* LoadPointLight()
        {
            try
            {
                uint lightId = LightSystem_AllocateLight(LightTypeEnum.kDirectionalLight);
                return MemoryPoolSystem.UpdatePointLight(lightId);
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.ToString());
                MessageBox.Show(ex.ToString());
                return null;
            }
        }

        public static IntPtr GetDirectionalLight(uint lightId)
        {
            IntPtr ptr = LightSystem_GetDirectionalLight(lightId);
            if (ptr == IntPtr.Zero)
            {
                System.Diagnostics.Debug.WriteLine($"Warning: directional light not found");
            }
            return ptr;
        }

        public static IntPtr GetPointLight(uint lightId)
        {
            IntPtr ptr = LightSystem_GetPointLight(lightId);
            if (ptr == IntPtr.Zero)
            {
                System.Diagnostics.Debug.WriteLine($"Warning: point light not found");
            }
            return ptr;
        }

        public static ref PointLight GetPointLightById(uint lightId)
        {
            IntPtr ptr = LightSystem_GetPointLight(lightId);
            if (ptr == IntPtr.Zero)
            {
                return ref Unsafe.NullRef<PointLight>();
            }
            return ref Unsafe.AsRef<PointLight>(ptr.ToPointer());
        }

        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern uint LightSystem_LoadLight([MarshalAs(System.Runtime.InteropServices.UnmanagedType.LPStr)] string lightJson);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern uint LightSystem_AllocateLight(LightTypeEnum lightType);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern IntPtr LightSystem_GetDirectionalLight(uint directionalLightId);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern IntPtr LightSystem_GetPointLight(uint pointLightId);
    }
}
