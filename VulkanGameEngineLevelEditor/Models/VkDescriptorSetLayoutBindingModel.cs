using Newtonsoft.Json;
using System.Runtime.InteropServices;
using Vulkan;
using VulkanGameEngineLevelEditor.LevelEditor.Attributes;


namespace VulkanGameEngineLevelEditor.Models
{
    public unsafe struct VkDescriptorSetLayoutBindingModel
    {
        [Tooltip("Specifies the binding number for the descriptor set layout.")]
        public uint binding { get; set; }

        [Tooltip("Defines the type of descriptor for this binding.")]
        public VkDescriptorType descriptorType { get; set; }

        public VkDescriptorSetLayoutBindingModel() { }
    }
}
