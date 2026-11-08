using GlmSharp;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Text;
using System.Threading.Tasks;

namespace VulkanEngineCS.Models
{
    public class Animation2D
    {
        public uint AnimationId { get; set; }
        public List<ivec2> FrameList { get; set; } = new List<ivec2>();
        public float FrameHoldTime { get; set; }
    }
}
