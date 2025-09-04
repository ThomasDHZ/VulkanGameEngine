using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using VulkanGameEngineLevelEditor.GameEngine.Systems;

namespace VulkanGameEngineLevelEditor.GameEngine.Structs
{
    public class LevelLoader
    {
        public string LevelID { get; set; }
        public List<string> RenderPipelineList { get; set; }
        public List<string> LoadTextures { get; set; }
        public List<string> LoadMaterials { get; set; }
        public List<string> LoadSpriteVRAM { get; set; }
        public List<string> LoadTileSetVRAM { get; set; }
        public List<GameObjectLoader> GameObjectList { get; set; }
        public string LoadLevelLayout { get; set; }
    }

}
