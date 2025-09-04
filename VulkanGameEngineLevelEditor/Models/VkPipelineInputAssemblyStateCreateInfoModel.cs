using Newtonsoft.Json;
using System;
using System.Runtime.InteropServices;
using Vulkan;
using VulkanGameEngineLevelEditor.LevelEditor.Attributes;

namespace VulkanGameEngineLevelEditor.Models
{
    [StructLayout(LayoutKind.Sequential, Pack = 4)]
    public unsafe struct VkPipelineInputAssemblyStateCreateInfoModel
    {
        [IgnoreProperty]
        [Tooltip("Specifies the Vulkan structure type. Must be set to the pipeline input assembly state creation type.")]
        public VkStructureType sType { get; set; } = VkStructureType.VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;

        [Tooltip("Defines the primitive topology for vertex data, such as triangles or lines.")]
        public VkPrimitiveTopology topology { get; set; }

        [Tooltip("Enables or disables primitive restart for indexed draws.")]
        public bool primitiveRestartEnable { get; set; }

        [JsonIgnore]
        [IgnoreProperty]
        [Tooltip("Reserved for future use. Currently set to 0, as no flags are defined.")]
        public uint flags { get; set; } = 0;

        [JsonIgnore]
        [IgnoreProperty]
        [Tooltip("Pointer to an extension structure for additional input assembly state parameters. Typically null unless using extensions.")]
        public IntPtr pNext { get; set; }
        public VkPipelineInputAssemblyStateCreateInfoModel()
        {
        }
    }
}
