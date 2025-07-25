using Silk.NET.Vulkan;
using System.Runtime.InteropServices;
using Vulkan;


namespace VulkanGameEngineLevelEditor.Models
{
    public unsafe class VkOffset2DModel
    {
        public int x { get; set; }
        public int y { get; set; }

        public VkOffset2DModel() { }
        public VkOffset2DModel(int x, int y)
        {
            this.x = x;
            this.y = y;
        }
    }
}
