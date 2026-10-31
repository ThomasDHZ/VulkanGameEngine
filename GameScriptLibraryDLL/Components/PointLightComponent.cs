using GlmSharp;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace GameScriptLibraryDLL.Components
{
    [System.Runtime.CompilerServices.InlineArray(6)]
    public struct PointLightShadowMat4Array
    {
        private mat4 _element0;
    }

    [StructLayout(LayoutKind.Explicit, Size = 448)]
    public unsafe struct PointLight
    {
        [FieldOffset(0)] public vec3 LightPosition;
        [FieldOffset(16)] public vec3 LightColor;
        [FieldOffset(32)] public float LightRadius;
        [FieldOffset(36)] public float LightIntensity;
        [FieldOffset(40)] public float ShadowStrength;
        [FieldOffset(44)] public float ShadowBias;
        [FieldOffset(48)] public float ShadowSoftness;
        [FieldOffset(52)] public int LightLayer;
        [FieldOffset(56)] public uint LightActive;
        [FieldOffset(64)] public PointLightShadowMat4Array LightSpaceMatrix;
    }

    [StructLayout(LayoutKind.Sequential, Pack = 4)]
    public struct PointLightComponent
    {
        public uint PointLightMemoryPoolIndex { get; set; } = uint.MaxValue;
        public uint PointLightCameraId { get; set; } = uint.MaxValue;
        public bool DirtyFlag { get; set; } = true;
        public PointLightComponent()
        {
        }
    };
}
