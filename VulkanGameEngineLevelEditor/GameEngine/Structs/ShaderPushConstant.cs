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
    public unsafe struct ShaderPushConstant
    {
        public fixed char Name[256];
        public size_t PushConstantSize { get; set; }
        public size_t PushConstantVariableListCount { get; set; }
        public VkShaderStageFlagBits ShaderStageFlags { get; set; }
        public ShaderVariable* PushConstantVariableList { get; set; }
        public void* PushConstantBuffer { get; set; }
        public bool GlobalPushContant { get; set; }

        public void SetName(string name)
        {
            if (string.IsNullOrEmpty(name))
                return;

            fixed (char* ptr = Name)
            {
                int length = Math.Min(name.Length, 255);
                for (int i = 0; i < length; i++)
                {
                    ptr[i] = name[i];
                }
                ptr[length] = '\0'; // Null-terminate
            }
        }

        public string GetName()
        {
            fixed (char* ptr = Name)
            {
                return new string(ptr);
            }
        }
    }
}
