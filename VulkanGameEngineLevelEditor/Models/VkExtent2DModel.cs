using System.Runtime.InteropServices;
using Vulkan;

namespace VulkanGameEngineLevelEditor.Models
{
    public unsafe class VkExtent2DModel
    {
        public uint width { get; set; }
        public uint height { get; set; }

        public VkExtent2DModel()
        {
        }
    }
}
