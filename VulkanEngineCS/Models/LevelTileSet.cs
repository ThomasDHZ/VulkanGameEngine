using GlmSharp;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VulkanEngineCS.Models
{
    public struct LevelTileSet
    {
        public Guid TileSetId { get; set; } = new Guid();
        public Guid MaterialId { get; set; } = new Guid();
        public vec2 TilePixelSize { get; set; } = new vec2();
        public ivec2 TileSetBounds { get; set; } = new ivec2(); 
        public vec2 TileScale { get; set; } = new vec2(5.0f);
        public vec2 TileUVSize { get; set; } = new vec2();
        public List<Tile> TileList { get; set; } = new List<Tile>();

        public LevelTileSet()
        {
        }
    }
}
