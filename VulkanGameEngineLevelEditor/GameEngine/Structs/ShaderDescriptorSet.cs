using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Vulkan;

namespace VulkanGameEngineLevelEditor.GameEngine.Structs
{
    public unsafe struct ShaderDescriptorSet
    {

        public fixed char Name[256];
        public uint Binding { get; set; } = 0;
        public VkDescriptorType DescripterType { get; set; }
        public size_t ShaderStructListCount { get; set; } = 0;
        public ShaderStruct* ShaderStructList { get; set; } = null;
        public ShaderDescriptorSet()
        {
        }
    }
}
