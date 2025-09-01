using Silk.NET.Vulkan;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Vulkan;

namespace VulkanGameEngineLevelEditor.GameEngine.Structs
{
    public unsafe struct ShaderPushConstant
    {
        public string PushConstantName { get; set; } = string.Empty;
        public size_t PushConstantSize { get; set; } = 0;
        public size_t PushConstantVariableListCount { get; set; } = 0;
        public VkShaderStageFlagBits ShaderStageFlags { get; set; }
        public ShaderVariable* PushConstantVariableList { get; set; } = null;
        public void* PushConstantBuffer { get; set; } = null;
        public bool GlobalPushContant { get; set; } = false;

        public ShaderPushConstant()
        {
        }
    }
}
