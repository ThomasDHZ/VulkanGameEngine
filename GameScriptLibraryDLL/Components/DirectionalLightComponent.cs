using GlmSharp;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace GameScriptLibraryDLL.Components
{
    [StructLayout(LayoutKind.Explicit, Size = 116)]
    public struct DirectionalLight
    {
        [FieldOffset(0)] public vec3 LightColor;
        [FieldOffset(16)] public vec3 LightDirection;
        [FieldOffset(32)] public float LightIntensity;
        [FieldOffset(36)] public float ShadowStrength;
        [FieldOffset(40)] public float ShadowBias;
        [FieldOffset(44)] public float ShadowSoftness;
        [FieldOffset(48)] public uint LightActive;
        [FieldOffset(52)] public mat4 LightSpaceMatrix;
    }

    [StructLayout(LayoutKind.Sequential, Pack = 4)]
    public struct DirectionalLightComponent
    {
        public uint DirectionalLightMemoryPoolIndex { get; set; } = uint.MaxValue;
        public uint DirectionalLightCameraId { get; set; } = uint.MaxValue;
        public DirectionalLightComponent()
        {
        }
    };
}
