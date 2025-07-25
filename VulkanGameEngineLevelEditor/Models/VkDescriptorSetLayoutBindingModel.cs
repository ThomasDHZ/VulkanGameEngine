using Newtonsoft.Json;
using System.Runtime.InteropServices;
using Vulkan;


namespace VulkanGameEngineLevelEditor.Models
{
    public unsafe struct VkDescriptorSetLayoutBindingModel
    {
        public uint binding { get; set; }
        public VkDescriptorType descriptorType { get; set; }
        public uint descriptorCount { get; set; }
        public VkShaderStageFlagBits stageFlags { get; set; }
        [JsonIgnore]
        public VkSampler* pImmutableSamplers { get; set; }

        public VkDescriptorSetLayoutBindingModel() { }
    }
}
