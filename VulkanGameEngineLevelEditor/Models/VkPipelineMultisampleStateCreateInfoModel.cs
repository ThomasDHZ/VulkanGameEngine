using Newtonsoft.Json;
using System;
using System.ComponentModel;
using System.Runtime.InteropServices;
using Vulkan;


namespace VulkanGameEngineLevelEditor.Models
{
    [StructLayout(LayoutKind.Sequential, Pack = 4)]
    public unsafe struct VkPipelineMultisampleStateCreateInfoModel
    {
        [IgnoreProperty]
        public VkStructureType sType { get; set; } = VkStructureType.VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        [DisplayNameAttribute("Sample Count")]
        public VkSampleCountFlagBits rasterizationSamples { get; set; }
        [DisplayNameAttribute("Sample Shading Enable")]
        public bool sampleShadingEnable { get; set; }
        [DisplayNameAttribute("Min Sample Shading")]
        public float minSampleShading { get; set; }
        [JsonIgnore]
        [IgnoreProperty]
        public uint* pSampleMask { get; set; } = null;
        public bool alphaToCoverageEnable { get; set; }
        public bool alphaToOneEnable { get; set; }
        [IgnoreProperty] 
        public uint flags { get; set; } = 0;
        [JsonIgnore]
        [IgnoreProperty]
        public IntPtr pNext { get; set; }

        public VkPipelineMultisampleStateCreateInfoModel()
        {
        }
    }
}
