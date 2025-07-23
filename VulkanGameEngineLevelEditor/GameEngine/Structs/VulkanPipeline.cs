using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using Vulkan;

namespace VulkanGameEngineLevelEditor.GameEngine.Structs
{
    [StructLayout(LayoutKind.Sequential, Pack = 8)]
    public unsafe struct VulkanPipeline
    {
        public Guid PipelineId = Guid.Empty;
        public size_t DescriptorSetLayoutCount { get; set; } = 0;
        public size_t DescriptorSetCount { get; set; } = 0;
        public VkDescriptorPool DescriptorPool { get; set; } = VulkanCSConst.VK_NULL_HANDLE;
        public VkDescriptorSetLayout* DescriptorSetLayoutList { get; set; } = null;
        public VkDescriptorSet* DescriptorSetList { get; set; } = null;
        public VkPipeline Pipeline { get; set; } = VulkanCSConst.VK_NULL_HANDLE;
        public VkPipelineLayout PipelineLayout { get; set; } = VulkanCSConst.VK_NULL_HANDLE;
        public VkPipelineCache PipelineCache { get; set; } = VulkanCSConst.VK_NULL_HANDLE;
        public VulkanPipeline()
        {
        }

    };
}
