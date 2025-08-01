using Newtonsoft.Json;
using System;
using System.ComponentModel;
using System.Runtime.InteropServices;
using Vulkan;
using VulkanGameEngineLevelEditor.LevelEditor.Attributes;


namespace VulkanGameEngineLevelEditor.Models
{
    [StructLayout(LayoutKind.Sequential, Pack = 4)]
    public unsafe struct VkPipelineMultisampleStateCreateInfoModel
    {
        [IgnoreProperty]
        [Tooltip("Specifies the Vulkan structure type. Must be set to the pipeline multisample state creation type.")]
        public VkStructureType sType { get; set; } = VkStructureType.VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;

        [DisplayNameAttribute("Sample Count")]
        [Tooltip("Defines the number of samples per pixel for rasterization.")]
        public VkSampleCountFlagBits rasterizationSamples { get; set; }

        [DisplayNameAttribute("Sample Shading Enable")]
        [Tooltip("Enables or disables sample shading for finer multisampling control.")]
        public bool sampleShadingEnable { get; set; }

        [DisplayNameAttribute("Min Sample Shading")]
        [Tooltip("Sets the minimum fraction of samples to shade when sample shading is enabled.")]
        public float minSampleShading { get; set; }

        [JsonIgnore]
        [IgnoreProperty]
        [Tooltip("Pointer to a sample mask array controlling which samples are active. Typically null for default behavior.")]
        public uint* pSampleMask { get; set; } = null;

        [Tooltip("Enables or disables alpha-to-coverage for multisampled rendering.")]
        public bool alphaToCoverageEnable { get; set; }

        [Tooltip("Enables or disables setting alpha to one for multisampled rendering.")]
        public bool alphaToOneEnable { get; set; }

        [IgnoreProperty]
        [Tooltip("Reserved for future use. Currently set to 0, as no flags are defined.")]
        public uint flags { get; set; } = 0;

        [JsonIgnore]
        [IgnoreProperty]
        [Tooltip("Pointer to an extension structure for additional multisample state parameters. Typically null unless using extensions.")]
        public IntPtr pNext { get; set; }

        public VkPipelineMultisampleStateCreateInfoModel()
        {
        }
    }
}
