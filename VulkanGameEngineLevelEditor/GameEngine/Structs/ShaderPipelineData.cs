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
        public nuint ShaderCount { get; set; }
        public nuint DescriptorBindingCount { get; set; }
        public nuint VertexInputBindingCount { get; set; }
        public nuint VertexInputAttributeListCount { get; set; }
        public nuint PushConstantCount { get; set; }
        public ShaderDescriptorBinding* DescriptorBindingsList { get; set; }
        public VkVertexInputBindingDescription* VertexInputBindingList { get; set; }
        public VkVertexInputAttributeDescription* VertexInputAttributeList { get; set; }
        public ShaderPushConstant* PushConstantList { get; set; }
        public IntPtr ShaderList { get; set; }

        public ShaderPipelineData()
        {
            ShaderCount = 0;
            DescriptorBindingCount = 0;
            VertexInputBindingCount = 0;
            VertexInputAttributeListCount = 0;
            PushConstantCount = 0;
            ShaderList = IntPtr.Zero;
            DescriptorBindingsList = null;
            VertexInputBindingList = null;
            VertexInputAttributeList = null;
            PushConstantList = null;
        }

        public string[] GetShaderList()
        {
            if (ShaderList == IntPtr.Zero || ShaderCount == 0)
                return Array.Empty<string>();

            string[] result = new string[ShaderCount];
            for (int i = 0; i < (int)ShaderCount; i++)
            {
                IntPtr stringPtr = Marshal.ReadIntPtr(ShaderList, i * IntPtr.Size);
                result[i] = stringPtr != IntPtr.Zero ? Marshal.PtrToStringAnsi(stringPtr) : string.Empty;
            }
            return result;
        }
    }
}
