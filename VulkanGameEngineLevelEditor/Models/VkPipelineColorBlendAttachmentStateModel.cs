using Silk.NET.Core;
using System.Runtime.InteropServices;
using Vulkan;


namespace VulkanGameEngineLevelEditor.Models
{
    public unsafe class VkPipelineColorBlendAttachmentStateModel
    {
        public Bool32 blendEnable { get; set; }
        public VkBlendFactor srcColorBlendFactor { get; set; }
        public VkBlendFactor dstColorBlendFactor { get; set; }
        public VkBlendOp colorBlendOp { get; set; }
        public VkBlendFactor srcAlphaBlendFactor { get; set; }
        public VkBlendFactor dstAlphaBlendFactor { get; set; }
        public VkBlendOp alphaBlendOp { get; set; }
        public VkColorComponentFlagBits colorWriteMask { get; set; }

        public VkPipelineColorBlendAttachmentStateModel() { }
    }
}
