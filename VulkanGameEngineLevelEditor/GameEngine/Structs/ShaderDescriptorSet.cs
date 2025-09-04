using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using Vulkan;

namespace VulkanGameEngineLevelEditor.GameEngine.Structs
{
    [StructLayout(LayoutKind.Sequential, Pack = 8)]
    public unsafe struct ShaderDescriptorSet
    {
        public IntPtr Name;
        public uint Binding;
        public VkDescriptorType DescripterType;
        public nuint ShaderStructListCount;
        public ShaderStruct* ShaderStructList;

        public string GetName() => Marshal.PtrToStringAnsi(Name) ?? string.Empty;
    }
}
