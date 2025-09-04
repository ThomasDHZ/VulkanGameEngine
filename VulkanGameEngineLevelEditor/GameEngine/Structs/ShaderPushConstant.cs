using Silk.NET.Vulkan;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using Vulkan;
using VulkanGameEngineLevelEditor.GameEngine.Systems;

namespace VulkanGameEngineLevelEditor.GameEngine.Structs
{
    [StructLayout(LayoutKind.Sequential, Pack = 8)]
    public unsafe struct ShaderPushConstant
    {
        public IntPtr PushConstantName; 
        public nuint PushConstantSize;
        public nuint PushConstantVariableListCount;
        public VkShaderStageFlagBits ShaderStageFlags;
        public ShaderVariable* PushConstantVariableList;
        public void* PushConstantBuffer;
        public bool GlobalPushContant;

        public string GetName() => Marshal.PtrToStringAnsi(PushConstantName) ?? string.Empty;
    }
}
