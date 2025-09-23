using GlmSharp;
using Newtonsoft.Json;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using VulkanGameEngineLevelEditor.Models;

namespace VulkanGameEngineLevelEditor.GameEngineAPI
{
    [StructLayout(LayoutKind.Sequential, Pack = 4)]
    public unsafe struct Animation2D
    {
        public uint AnimationId { get; set; }
        public ivec2* FrameList { get; set; }
        public size_t FrameCount { get; set; }
        public float FrameHoldTime { get; set; }
    }
}
