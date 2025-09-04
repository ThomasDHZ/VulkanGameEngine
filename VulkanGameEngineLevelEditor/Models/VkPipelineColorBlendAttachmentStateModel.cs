using Silk.NET.Core;
using System.Runtime.InteropServices;
using Vulkan;


namespace VulkanGameEngineLevelEditor.Models
{
    public unsafe class VkPipelineColorBlendAttachmentStateModel
    {
        public bool BlendEnable { get; set; }
        public VkBlendFactor SrcColorBlendFactor { get; set; }
        public VkBlendFactor DstColorBlendFactor { get; set; }
        public VkBlendOp ColorBlendOp { get; set; }
        public VkBlendFactor SrcAlphaBlendFactor { get; set; }
        public VkBlendFactor DstAlphaBlendFactor { get; set; }
        public VkBlendOp AlphaBlendOp { get; set; }
        public VkColorComponentFlagBits ColorWriteMask { get; set; }

        public VkPipelineColorBlendAttachmentStateModel() { }
    }
}
