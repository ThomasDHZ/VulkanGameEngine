using GlmSharp;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace GameScriptLibraryDLL.Components
{
    public struct DirectionalLightComponent
    {
        public vec3 LightColor { get; set; } = new vec3(1.0f, 1.0f, 1.0f);
        public vec3 LightDirection { get; set; } = new vec3(0.3f, 0.3f, 1.0f);
        public float LightIntensity { get; set; } = 1.5f;
        public float ShadowStrength { get; set; } = 1.0f;
        public float ShadowBias { get; set; } = 0.012f;
        public float ShadowSoftness { get; set; } = 0.008f;
        public DirectionalLightComponent()
        {
        }
    };
}
