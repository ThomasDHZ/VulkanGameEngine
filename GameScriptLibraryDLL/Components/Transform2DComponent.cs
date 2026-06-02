using GlmSharp;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Text;
using System.Threading.Tasks;

namespace GameScriptLibraryDLL.Components
{
    public struct Transform2DComponent
    {
        public vec2 Position { get; set; } = new vec2(0.0f);
        public vec2 Rotation { get; set; } = new vec2(0.0f);
        public vec2 Scale { get; set; } = new vec2(1.0f, 1.0f);
        public bool Dirty { get; set; } = true;
        public Transform2DComponent()
        {
        }
    }
}
