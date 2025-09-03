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
    public unsafe struct ShaderPipelineData
    {
        public nuint ShaderCount { get; set; } = 0;
        public nuint DescriptorBindingCount { get; set; } = 0;
        public nuint VertexInputBindingCount { get; set; } = 0;
        public nuint VertexInputAttributeListCount { get; set; } = 0;
        public nuint PushConstantCount { get; set; } = 0;
        public IntPtr ShaderList { get; set; } = IntPtr.Zero;
        public ShaderDescriptorBinding* DescriptorBindingsList { get; set; } = null;
        public VkVertexInputBindingDescription* VertexInputBindingList { get; set; } = null;
        public VkVertexInputAttributeDescription* VertexInputAttributeList { get; set; } = null;
        public ShaderPushConstant* PushConstantList { get; set; } = null;

        public ShaderPipelineData()
        {
        }

        public string[] GetShaderList()
        {
            if (ShaderList == IntPtr.Zero || ShaderCount == 0)
                return Array.Empty<string>();

            string[] result = new string[ShaderCount];
            for (int i = 0; i < (int)ShaderCount; i++)
            {
                IntPtr stringPtr = Marshal.ReadIntPtr(ShaderList, (int)(i * IntPtr.Size));
                result[i] = stringPtr != IntPtr.Zero ? Marshal.PtrToStringAnsi(stringPtr) : null;
            }
            return result;
        }
    }
}
