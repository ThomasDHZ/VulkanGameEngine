using Newtonsoft.Json;
using System;
using System.Runtime.InteropServices;
using Vulkan;
using VulkanGameEngineLevelEditor.LevelEditor.Attributes;

namespace VulkanGameEngineLevelEditor.Models
{
    [StructLayout(LayoutKind.Sequential, Pack = 4)]
    public unsafe struct VkPipelineDepthStencilStateCreateInfoModel
    {
        [IgnoreProperty]
        [Tooltip("Specifies the Vulkan structure type. Must be set to the pipeline depth-stencil state creation type.")]
        public VkStructureType sType { get; set; } = VkStructureType.VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;

        [Tooltip("Enables or disables depth testing for fragments.")]
        public VkBool32 depthTestEnable { get; set; }

        [Tooltip("Enables or disables writing depth values to the depth buffer.")]
        public VkBool32 depthWriteEnable { get; set; }

        [Tooltip("Specifies the comparison operation for depth testing.")]
        public VkCompareOp depthCompareOp { get; set; }

        [Tooltip("Enables or disables depth bounds testing to restrict fragment depth values.")]
        public VkBool32 depthBoundsTestEnable { get; set; }

        [Tooltip("Enables or disables stencil testing for fragments.")]
        public VkBool32 stencilTestEnable { get; set; }

        [Tooltip("Defines stencil operations and parameters for front-facing polygons.")]
        public VkStencilOpStateModel front { get; set; } = new VkStencilOpStateModel();

        [Tooltip("Defines stencil operations and parameters for back-facing polygons.")]
        public VkStencilOpStateModel back { get; set; } = new VkStencilOpStateModel();

        [Tooltip("Sets the minimum depth value for depth bounds testing.")]
        public float minDepthBounds { get; set; }

        [Tooltip("Sets the maximum depth value for depth bounds testing.")]
        public float maxDepthBounds { get; set; }

        [IgnoreProperty]
        [Tooltip("Reserved for future use. Currently set to 0, as no flags are defined.")]
        public uint flags { get; set; } = 0;

        [JsonIgnore]
        [IgnoreProperty]
        [Tooltip("Pointer to an extension structure for additional depth-stencil state parameters. Typically null unless using extensions.")]
        public IntPtr pNext { get; set; }

        public VkPipelineDepthStencilStateCreateInfoModel() { }
    }
}
