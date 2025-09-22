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
    public unsafe struct ShaderDescriptorBindingDLL
    {
        public IntPtr Name;
        public uint Binding;
        public nuint DescriptorCount;
        public VkShaderStageFlagBits ShaderStageFlags;
        public DescriptorBindingPropertiesEnum DescriptorBindingType;
        public VkDescriptorType DescripterType;
        public VkDescriptorImageInfo* DescriptorImageInfo;
        public VkDescriptorBufferInfo* DescriptorBufferInfo;

        public string GetName() => Marshal.PtrToStringAnsi(Name) ?? string.Empty;
    }
}
