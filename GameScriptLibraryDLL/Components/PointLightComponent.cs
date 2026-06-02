using GlmSharp;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace GameScriptLibraryDLL.Components
{
    public struct PointLightComponent
    {
        public vec3 LightPosition { get; set; } = new vec3(0.0f);
        public vec3 LightColor { get; set; } = new vec3(1.0f, 0.95f, 0.8f);
        public float LightRadius { get; set; } = 200.0f;
        public float LightIntensity { get; set; } = 2.0f;
        public float ShadowStrength { get; set; } = 1.0f;
        public float ShadowBias { get; set; } = 0.012f;
        public float ShadowSoftness { get; set; } = 0.008f;
        public PointLightComponent()
        {
        }
    };
}
