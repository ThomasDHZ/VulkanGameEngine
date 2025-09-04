using Newtonsoft.Json;
using System;
using System.Runtime.InteropServices;
using Vulkan;
using VulkanGameEngineLevelEditor.LevelEditor.Attributes;


namespace VulkanGameEngineLevelEditor.Models
{
    [StructLayout(LayoutKind.Sequential, Pack = 4)]
    public unsafe struct VkPipelineRasterizationStateCreateInfoModel
    {
        [IgnoreProperty]
        [Tooltip("Specifies the Vulkan structure type. Must be set to the pipeline rasterization state creation type.")]
        public VkStructureType sType { get; set; } = VkStructureType.VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;

        [Tooltip("Enables or disables depth clamping to restrict depth values to the viewport range.")]
        public bool depthClampEnable { get; set; }

        [Tooltip("Enables or disables discarding of rasterized fragments before fragment shading.")]
        public bool rasterizerDiscardEnable { get; set; }

        [Tooltip("Defines how polygons are rendered, such as filled, wireframe, or points.")]
        public VkPolygonMode polygonMode { get; set; }

        [Tooltip("Specifies which polygon faces are culled during rasterization.")]
        public VkCullModeFlagBits cullMode { get; set; }

        [Tooltip("Defines which polygon orientation is considered front-facing.")]
        public VkFrontFace frontFace { get; set; }

        [Tooltip("Enables or disables depth bias to adjust depth values for polygons.")]
        public bool depthBiasEnable { get; set; }

        [Tooltip("Sets the constant factor for depth bias calculations.")]
        public float depthBiasConstantFactor { get; set; }

        [Tooltip("Sets the maximum or minimum depth bias value when clamping is enabled.")]
        public float depthBiasClamp { get; set; }

        [Tooltip("Sets the slope factor for depth bias based on polygon slope.")]
        public float depthBiasSlopeFactor { get; set; }

        [Tooltip("Specifies the width of rasterized lines in pixels.")]
        public float lineWidth { get; set; }

        [IgnoreProperty]
        [Tooltip("Reserved for future use. Currently set to 0, as no flags are defined.")]
        public uint flags { get; set; } = 0;

        [JsonIgnore]
        [IgnoreProperty]
        [Tooltip("Pointer to an extension structure for additional rasterization state parameters. Typically null unless using extensions.")]
        public IntPtr pNext { get; set; }
        public VkPipelineRasterizationStateCreateInfoModel() { }
    }
}
