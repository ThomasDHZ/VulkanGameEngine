using GlmSharp;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace VulkanEngineCS
{
    [StructLayout(LayoutKind.Sequential)]
    public struct InteropIvec2
    {
        public int x;
        public int y;

        public InteropIvec2(ivec2 v) { x = v.x; y = v.y; }
        public ivec2 ToGlm() => new ivec2(x, y);
    }
}
