using Newtonsoft.Json;
using Silk.NET.Vulkan;
using System;
using System.Runtime.InteropServices;
using Vulkan;
using VulkanGameEngineLevelEditor.LevelEditor.Attributes;


namespace VulkanGameEngineLevelEditor.Models
{
    [StructLayout(LayoutKind.Sequential, Pack = 4)]
    public unsafe struct VkPipelineColorBlendStateCreateInfoModel
    {
        [IgnoreProperty]
        [Tooltip("Specifies the Vulkan structure type. Must be set to the pipeline color blend state creation type.")]
        public VkStructureType sType { get; set; } = VkStructureType.VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;

        [JsonIgnore]
        [IgnoreProperty]
        [Tooltip("Pointer to an extension structure for additional color blend state parameters. Typically null unless using extensions.")]
        public IntPtr pNext { get; set; }

        [JsonIgnore]
        [IgnoreProperty]
        [Tooltip("Reserved for future use. Currently set to 0, as no flags are defined.")]
        public VkPipelineColorBlendStateCreateFlagBits flags { get; set; }

        [Tooltip("Enables or disables logical operations for color blending.")]
        public VkBool32 logicOpEnable { get; set; }

        [Tooltip("Specifies the logical operation to apply when logical operations are enabled.")]
        public VkLogicOp logicOp { get; set; }

        [Tooltip("Sets the number of color blend attachment states in the attachments array.")]
        public uint attachmentCount { get; set; }

        [Tooltip("Pointer to an array of color blend attachment states defining per-attachment blending.")]
        public VkPipelineColorBlendAttachmentState* pAttachments { get; set; }

        //   [Tooltip("Specifies the four blend constants used for certain blending operations.")]
        public fixed float blendConstants[4];

        public VkPipelineColorBlendStateCreateInfoModel()
        {
            sType = VkStructureType.VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            pNext = IntPtr.Zero;
            flags = 0;
            logicOpEnable = Vk.False;
            logicOp = VkLogicOp.VK_LOGIC_OP_CLEAR;
            attachmentCount = 0;
            pAttachments = null;
            blendConstants[0] = 0.0f;
            blendConstants[1] = 0.0f;
            blendConstants[2] = 0.0f;
            blendConstants[3] = 0.0f;
        }

    }
}
