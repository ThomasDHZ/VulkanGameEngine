using GlmSharp;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using VulkanEngineCS.Enum;

namespace VulkanEngineCS.Models
{
    public struct Tile
    {
        public uint TileId { get; set; } = 0;
        public ivec2 TileUVCellOffset { get; set; } = new ivec2();
        public vec2 TileUVOffset { get; set; } = new vec2();
        public int TileLayer { get; set; } = 0;
        public TileColliderTypeEnum TileCollider { get; set; } = TileColliderTypeEnum.kTileColliderNone;
        public bool IsAnimatedTile { get; set; } = false;
        public Tile()
        {
        }
    }
}
