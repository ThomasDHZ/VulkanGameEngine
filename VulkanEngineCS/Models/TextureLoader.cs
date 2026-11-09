using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using VulkanCS;
using VulkanEngineCS.Enum;

namespace VulkanEngineCS.Models
{
    public struct TextureLoader
    {
        public List<string> TextureFilePath { get; set; } = new List<string>();
        public Guid TextureId = Guid.Empty;
        public VkFormat TextureByteLayout { get; set; }
        public TextureTypeEnum TextureType { get; set; }
        public TextureUsageTypeEnum TextureUsageType { get; set; }
        public VkSamplerCreateInfo SamplerCreateInfo { get; set; }
        public uint MipMapCount { get; set; } = 0;
        public TextureLoader()
        {
        }
    }
}
