using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VulkanEngineCS.Models
{
    public class LevelLoader
    {
        public Guid LevelId { get; set; } = Guid.Empty;
        public string SkyBox {  get; set; } = string.Empty;
        public List<string> Materials { get; set; } = new List<string>();
        public List<string> SpriteVram { get; set; }= new List<string>();
        public string TileSetVram { get; set; } = string.Empty;
        public List<string> GameObjectBehaviors { get; set; } = new List<string>();
        public Guid PresentingAttachmentTextureId { get; set; } = Guid.Empty;
        public string LoadLevelLayout { get; set; } = string.Empty;
        public List<string> MainRenderPassList { get; set; } = new List<string>();
        public List<string> LevelEditorRenderPass { get; set; } = new List<string>();

    }
}
