﻿using GlmSharp;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VulkanGameEngineLevelEditor.Models
{
    public class Animation2DModel
    {
        public string AnimationName { get; set; }
        public float FrameHoldTime { get; set; }
        public List<ivec2> FrameList { get; set; }
    }
}
