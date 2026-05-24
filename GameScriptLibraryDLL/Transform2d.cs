using GlmSharp;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Text;
using System.Threading.Tasks;

namespace GameScriptLibraryDLL
{
    public class Transform2d : Component
    {
        public vec2 Position { get; set; }
        public vec2 Rotation { get; set; }
        public vec2 Scale { get; set; } = new vec2(1, 1);

        public void Move(float x, float y) { }
        public void Move(vec2 delta) => Move(delta.x, delta.y);
    }
}
