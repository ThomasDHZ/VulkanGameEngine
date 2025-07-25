using System.Runtime.InteropServices;
using Vulkan;


namespace VulkanGameEngineLevelEditor.Models
{
    public unsafe class VkStencilOpStateModel
    {
        public VkStencilOp failOp { get; set; } = 0;
        public VkStencilOp passOp { get; set; } = 0;
        public VkStencilOp depthFailOp { get; set; } = 0;
        public VkCompareOp compareOp { get; set; } = 0;
        public uint compareMask { get; set; } = 0;
        public uint writeMask { get; set; } = 0;
        public uint reference { get; set; } = 0;
        public VkStencilOpStateModel() { }
    }
}
