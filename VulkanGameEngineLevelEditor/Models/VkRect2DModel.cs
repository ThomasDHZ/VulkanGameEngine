using Silk.NET.Vulkan;
using System.Runtime.InteropServices;

namespace VulkanGameEngineLevelEditor.Models
{
    public unsafe class VkRect2DModel
    {
        public VkOffset2DModel offset { get; set; }
        public VkExtent2DModel extent { get; set; }

        public VkRect2DModel()
        {
            offset = new VkOffset2DModel();
            extent = new VkExtent2DModel();
        }

        public VkRect2DModel(VkOffset2DModel offset, VkExtent2DModel extent)
        {
            this.offset = offset;
            this.extent = extent;
        }

        public VkRect2DModel(Rect2D other)
        {
            offset = new VkOffset2DModel()
            {
                x = other.Offset.X,
                y = other.Offset.Y
            };
            extent = new VkExtent2DModel()
            {
                width = other.Extent.Width,
                height = other.Extent.Height
            };
        }
    }
}
