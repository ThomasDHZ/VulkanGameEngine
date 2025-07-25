using Silk.NET.Vulkan;

namespace VulkanGameEngineLevelEditor.Models
{
    public class VkViewportModel
    {
        public float x { get; set; }
        public float y { get; set; }
        public float width { get; set; }
        public float height { get; set; }
        public float minDepth { get; set; }
        public float maxDepth { get; set; }

        public VkViewportModel()
        { }
    }
}
