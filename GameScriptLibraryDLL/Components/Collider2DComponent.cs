using GlmSharp;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace GameScriptLibraryDLL.Components
{
    public struct Collider2DComponent
    {
        public ivec2 Size { get; set; } = new ivec2(32, 32);
        public ivec2 Offset { get; set; } = new ivec2(0, 0);
        public bool Enabled { get; set; } = true;
        public bool IsTrigger { get; set; } = false;
        public Collider2DComponent()
        {
        }
    }
}
