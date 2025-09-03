using Silk.NET.Vulkan;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using Vulkan;
using VulkanGameEngineLevelEditor.Models;

namespace VulkanGameEngineLevelEditor.GameEngine.Structs
{
    [StructLayout(LayoutKind.Sequential, Pack = 8)]
    public unsafe struct ShaderDescriptorBinding
    {
        public fixed char Name[256];
        public uint Binding { get; set; }
        public nuint DescriptorCount { get; set; }
        public VkShaderStageFlagBits ShaderStageFlags { get; set; }
        public DescriptorBindingPropertiesEnum DescriptorBindingType { get; set; }
        public VkDescriptorType DescripterType { get; set; }
        public VkDescriptorImageInfo* DescriptorImageInfo { get; set; }
        public VkDescriptorBufferInfo* DescriptorBufferInfo { get; set; }

        public ShaderDescriptorBinding()
        {
            Binding = 0;
            DescriptorCount = 0;
            ShaderStageFlags = 0;
            DescriptorBindingType = 0;
            DescripterType = 0;
            DescriptorImageInfo = null;
            DescriptorBufferInfo = null;
            // Initialize Name to empty null-terminated string
            fixed (char* ptr = Name)
            {
                ptr[0] = '\0';
            }
        }

        public void SetName(string name)
        {
            if (string.IsNullOrEmpty(name))
            {
                fixed (char* ptr = Name)
                {
                    ptr[0] = '\0';
                }
                return;
            }

            fixed (char* ptr = Name)
            {
                int length = Math.Min(name.Length, 255);
                for (int i = 0; i < length; i++)
                {
                    ptr[i] = name[i];
                }
                ptr[length] = '\0';
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
