using GlmSharp;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using VulkanEngineCoreCS.Enum;

namespace VulkanEngineCS.Models
{
    public struct SpriteVramLoader
    {
        public Guid VramSpriteId { get; set; } = Guid.Empty;
        public Guid MaterialId { get; set; } = Guid.Empty;
        public uint SpriteLayer { get; set; } = 0;
        public vec2 SpriteScale { get; set; } = new vec2(1.0f, 1.0f);
        public DrawFlagBits SpriteDrawFlags { get; set; } = DrawFlagBits.kDrawFlag_DrawInstance;
        public ivec2 SpritePixelSize { get; set; }  = new ivec2();
        public vec4 SpriteColor { get; set; } = new vec4(0.0f, 0.0f, 0.0f, 1.0f);
        public List<Animation2D> AnimationList = new List<Animation2D>();
        public SpriteVramLoader()
        {
        }
    };
}
