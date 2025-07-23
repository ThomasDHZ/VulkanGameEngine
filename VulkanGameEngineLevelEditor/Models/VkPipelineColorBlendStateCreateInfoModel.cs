using Newtonsoft.Json;
using Silk.NET.Vulkan;
using System;
using System.Runtime.InteropServices;
using Vulkan;


namespace VulkanGameEngineLevelEditor.Models
{
    [StructLayout(LayoutKind.Sequential, Pack = 4)]
    public unsafe struct VkPipelineColorBlendStateCreateInfoModel
    {
        [IgnoreProperty]
        public VkStructureType sType { get; set; } = VkStructureType.VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        [JsonIgnore]
        [IgnoreProperty]
        public IntPtr pNext { get; set; }
        [IgnoreProperty]
        public VkPipelineColorBlendStateCreateFlagBits flags { get; set; }
        public VkBool32 logicOpEnable { get; set; }
        public VkLogicOp logicOp { get; set; }
        [IgnoreProperty]
        public uint attachmentCount { get; set; }
        [JsonIgnore]
        [IgnoreProperty]
        public VkPipelineColorBlendAttachmentState* pAttachments { get; set; }
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

        public VkPipelineColorBlendStateCreateInfo Convert()
        {
            return new VkPipelineColorBlendStateCreateInfo
            {
                sType = sType,
                pNext = null,
                attachmentCount = attachmentCount,
                pAttachments = pAttachments,
                // blendConstants = blendConstants,
                flags = flags,
                logicOpEnable = logicOpEnable,
                logicOp = logicOp
            };
        }

        public void Dispose()
        {
        }
    }
}
