
using GlmSharp;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace GameScriptLibraryDLL
{
    public struct SpriteComponent
    {
        public uint GameObjectId { get; set; } = uint.MaxValue;
        public uint SpriteInstanceId { get; set; } = 0;
        public uint CurrentAnimationId { get; set; } = 0;
        public uint CurrentFrame { get; set; } = 0;
        public uint SpriteLayer { get; set; } = 0;
        public ivec2 FlipSprite { get; set; } = new ivec2(0);
        public Guid SpriteVramId { get; set; } = new Guid();
        public float CurrentFrameTime { get; set; } = 0.0f;
        public SpriteComponent()
        {
        }
    }
}
