using Silk.NET.Vulkan;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Vulkan;
using VulkanGameEngineLevelEditor.Models;

namespace VulkanGameEngineLevelEditor.GameEngine.Structs
{
    public unsafe struct ShaderDescriptorBinding
    {

        public fixed char Name[256];
        public uint Binding { get; set; } = 0;
        public size_t DescriptorCount { get; set; } = 0;
        public VkShaderStageFlagBits ShaderStageFlags { get; set; }
        public DescriptorBindingPropertiesEnum DescriptorBindingType { get; set; }
        public VkDescriptorType DescripterType { get; set; }
        public VkDescriptorImageInfo* DescriptorImageInfo { get; set; } = null;
        public VkDescriptorBufferInfo* DescriptorBufferInfo { get; set; } = null;
        public ShaderDescriptorBinding()
        {
        }
    }
}
