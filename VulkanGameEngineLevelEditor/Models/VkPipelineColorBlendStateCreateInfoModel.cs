using Newtonsoft.Json;
using Silk.NET.Vulkan;
using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using Vulkan;
using VulkanGameEngineLevelEditor.LevelEditor.Attributes;


namespace VulkanGameEngineLevelEditor.Models
{
    public struct BlendConstantsModel
    {
        public float Red { get; set; } = 0.0f;
        public float Green { get; set; } = 0.0f;
        public float Blue { get; set; } = 0.0f;
        public float Alpha { get; set; } = 0.0f;

        public BlendConstantsModel() { }
    }

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
        public bool LogicOpEnable { get; set; }

        [Tooltip("Specifies the logical operation to apply when logical operations are enabled.")]
        public VkLogicOp LogicOp { get; set; }

        [Tooltip("Sets the number of color blend attachment states in the attachments array.")]
        public uint AttachmentCount { get; set; }

        [IgnoreProperty]
        [Tooltip("Pointer to an array of color blend attachment states defining per-attachment blending.")]
        public IntPtr pAttachments { get; set; } = IntPtr.Zero;

        [Tooltip("Specifies the four blend constants used for certain blending operations.")]
        public BlendConstantsModel BlendConstants { get; set; } = new BlendConstantsModel();

        public VkPipelineColorBlendStateCreateInfoModel()
        {
            sType = VkStructureType.VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            pNext = IntPtr.Zero;
            flags = 0;
            LogicOpEnable = false;
            LogicOp = VkLogicOp.VK_LOGIC_OP_CLEAR;
            AttachmentCount = 0;
            pAttachments = pAttachments;
        }

    }
}
