using Newtonsoft.Json;
using System.Runtime.InteropServices;
using Vulkan;


namespace VulkanGameEngineLevelEditor.Models
{
    public unsafe struct VkDescriptorSetLayoutBindingModel
    {
        public uint binding { get; set; }
        public VkDescriptorType descriptorType { get; set; }
        public VkDescriptorSetLayoutBindingModel() { }
    }
}
