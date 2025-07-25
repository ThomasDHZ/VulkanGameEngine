using System;
using System.ComponentModel;
using System.Reflection;
using Vulkan;
using VulkanGameEngineLevelEditor.LevelEditor;
using VulkanGameEngineLevelEditor.LevelEditor.Attributes;
using VulkanGameEngineLevelEditor.LevelEditor.EditorEnhancements;


namespace VulkanGameEngineLevelEditor.Models
{
    [Serializable]
    public unsafe class VkSubpassDependencyModel
    {
        [TooltipAttribute("Specifies the index of the source subpass for the dependency.")]
        public uint SrcSubpass { get; set; }

        [TooltipAttribute("Specifies the index of the destination subpass for the dependency.")]
        public uint DstSubpass { get; set; }

        [TooltipAttribute("Defines the pipeline stages where the source subpass produces data.")]
        public VkPipelineStageFlagBits SrcStageMask { get; set; }

        [TooltipAttribute("Defines the pipeline stages where the destination subpass consumes data.")]
        public VkPipelineStageFlagBits DstStageMask { get; set; }

        [TooltipAttribute("Specifies the memory access types performed by the source subpass.")]
        public VkAccessFlagBits SrcAccessMask { get; set; }

        [TooltipAttribute("Specifies the memory access types performed by the destination subpass.")]
        public VkAccessFlagBits DstAccessMask { get; set; }

        [TooltipAttribute("Defines flags controlling the dependency's behavior, such as view locality.")]
        public VkDependencyFlagBits DependencyFlags { get; set; }

        public VkSubpassDependencyModel() : base()
        {
        }
    }
}