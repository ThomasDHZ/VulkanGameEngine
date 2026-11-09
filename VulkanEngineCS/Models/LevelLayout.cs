using GlmSharp;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VulkanEngineCS.Models
{
    public struct LevelLayout
    {
        public Guid LevelLayoutId { get; set; } = Guid.Empty;
        public ivec2 LevelBounds { get; set; }
        public ivec2 TileSizeInPixels { get; set; }
        public List<List<uint>> LevelLayoutLayerList { get; set; } = new List<List<uint>>();
        public LevelLayout()
        {
        }

    }
}
