using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Vulkan;

namespace VulkanGameEngineLevelEditor.GameEngine.Structs
{
    public unsafe struct ShaderPipelineData
    {
        public size_t ShaderCount { get; set; } = 0;
        public size_t DescriptorBindingCount { get; set; } = 0;
        public size_t ShaderStructCount { get; set; } = 0;
        public size_t VertexInputBindingCount { get; set; } = 0;
        public size_t VertexInputAttributeListCount { get; set; } = 0;
        public size_t ShaderOutputCount { get; set; } = 0;
        public size_t PushConstantCount { get; set; } = 0;
        public string* ShaderList { get; set; } = null;
        public ShaderDescriptorBinding* DescriptorBindingsList { get; set; } = null;
        public ShaderStruct* ShaderStructList { get; set; } = null;
        public VkVertexInputBindingDescription* VertexInputBindingList { get; set; } = null;
        public VkVertexInputAttributeDescription* VertexInputAttributeList { get; set; } = null;
        public ShaderVariable* ShaderOutputList { get; set; } = null;
        public ShaderPushConstant* PushConstantList { get; set; } = null;

        public ShaderPipelineData()
        {
        }
    }
}
