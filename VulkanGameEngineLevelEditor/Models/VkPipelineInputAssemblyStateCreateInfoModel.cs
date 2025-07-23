using Newtonsoft.Json;
using System.Runtime.InteropServices;
using Vulkan;

namespace VulkanGameEngineLevelEditor.Models
{
    [StructLayout(LayoutKind.Sequential, Pack = 4)]
    public unsafe struct VkPipelineInputAssemblyStateCreateInfoModel
    {
        [IgnoreProperty]
        public VkStructureType sType { get; set; } = VkStructureType.VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        public VkPrimitiveTopology topology { get; set; }
        public VkBool32 primitiveRestartEnable { get; set; }
        [IgnoreProperty]
        public uint flags { get; set; } = 0;
        [JsonIgnore]
        [IgnoreProperty]
        public void* pNext { get; set; } = null;
        public VkPipelineInputAssemblyStateCreateInfoModel()
        {
        }

        public VkPipelineInputAssemblyStateCreateInfo Convert()
        {
            return new VkPipelineInputAssemblyStateCreateInfo
            {
                sType = sType,
                topology = topology,
                primitiveRestartEnable = primitiveRestartEnable,
                flags = 0,
                pNext = null
            };
        }
    }
}
