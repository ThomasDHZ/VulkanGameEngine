using GlmSharp;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VulkanGameEngineLevelEditor.GameEngine.Components
{
    public struct SpriteComponent
    {
        [ReadOnly(true)]
        public Guid spriteVramId;

        [ReadOnly(true)]
        public uint currentAnimationId = 0;

        [ReadOnly(true)]
        public uint currentFrame = 0;

        [ReadOnly(true)]
        public float frameTimeAccumulator = 0.0f;

        [ReadOnly(true)]
        public bool flipX = false;

        [ReadOnly(true)]
        public bool flipY = false;

        [ReadOnly(true)]
        public int layer  = 0;

        [ReadOnly(true)]
        public vec4 tint = new vec4(1.0f);
        
        public SpriteComponent()
        {
        }
    };
}
