using GameScriptLibraryDLL.GameObjects;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using VulkanEngineCoreCS;

namespace VulkanEngineCS
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
            return DLLSystem.CallDLLFunc(() => LightSystem_LoadLight(lightJson));
        }

        public static DirectionalLight* LoadDirectionalLight()
        {
            uint lightId = DLLSystem.CallDLLFunc(() => LightSystem_AllocateLight(LightTypeEnum.kDirectionalLight));
            return MemoryPoolSystem.UpdateDirectionalLight(lightId);
        }

        public static PointLight* LoadPointLight()
        {
            uint lightId = DLLSystem.CallDLLFunc(() => LightSystem_AllocateLight(LightTypeEnum.kDirectionalLight));
            return MemoryPoolSystem.UpdatePointLight(lightId);
        }

        public static IntPtr GetDirectionalLight(uint lightId)
        {
            return DLLSystem.CallDLLFunc(() => LightSystem_GetDirectionalLight(lightId));
        }

        public static IntPtr GetPointLight(uint lightId)
        {
            return DLLSystem.CallDLLFunc(() => LightSystem_GetPointLight(lightId));
        }

        public static ref PointLight GetPointLightById(uint lightId)
        {
            IntPtr ptr = DLLSystem.CallDLLFunc(() => LightSystem_GetPointLight(lightId));
            if (ptr == IntPtr.Zero)
            {
                return ref Unsafe.NullRef<PointLight>();
            }
            return ref Unsafe.AsRef<PointLight>(ptr.ToPointer());
        }

        [DllImport("VulkanEngineInterop.dll", CallingConvention = CallingConvention.StdCall)] private static extern uint LightSystem_LoadLight([MarshalAs(System.Runtime.InteropServices.UnmanagedType.LPStr)] string lightJson);
        [DllImport("VulkanEngineInterop.dll", CallingConvention = CallingConvention.StdCall)] private static extern uint LightSystem_AllocateLight(LightTypeEnum lightType);
        [DllImport("VulkanEngineInterop.dll", CallingConvention = CallingConvention.StdCall)] private static extern IntPtr LightSystem_GetDirectionalLight(uint directionalLightId);
        [DllImport("VulkanEngineInterop.dll", CallingConvention = CallingConvention.StdCall)] private static extern IntPtr LightSystem_GetPointLight(uint pointLightId);
    }
}
