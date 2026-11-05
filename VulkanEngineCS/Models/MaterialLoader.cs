using GlmSharp;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using VulkanEngineCS.Enum;

namespace VulkanEngineCS.Models
{
    public class MaterialLoader
    {
        public Guid MaterialId { get; set; } = Guid.Empty;
        public TextureLoader AlbedoTexture { get; set; } = new TextureLoader();
        public TextureLoader NormalTexture { get; set; } = new TextureLoader();
        public TextureLoader MroTexture { get; set; } = new TextureLoader();
        public TextureLoader ClearCoatTexture { get; set; } = new TextureLoader();
        public TextureLoader SubSurfaceScatteringTexture { get; set; } = new TextureLoader();
        public TextureLoader SubSurfaceScatteringPropertiesTexture { get; set; } = new TextureLoader();
        public TextureLoader SheenTexture { get; set; } = new TextureLoader();
        public TextureLoader AnisotropyTexture { get; set; } = new TextureLoader();
        public TextureLoader EmissionTexture { get; set; } = new TextureLoader();
        public TextureLoader TranslucentTexture { get; set; } = new TextureLoader();
        public TextureLoader TranslucentPropertiesTexture { get; set; } = new TextureLoader();
        public uint ShadingModel { get; set; }
        public MaterialPropertiesEnum FeatureMask { get; set; }
        public vec3 ClearcoatTint { get; set; }
        public float IOR { get; set; }
        public float AlphaCutOff { get; set; }
    }
}
