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
        public size_t ShaderCount { get; set; }
        public size_t DescriptorBindingCount { get; set; }
        public size_t ShaderStructCount { get; set; }
        public size_t VertexInputBindingCount { get; set; }
        public size_t VertexInputAttributeListCount { get; set; }
        public size_t ShaderOutputCount { get; set; }
        public size_t PushConstantCount { get; set; }
        public IntPtr ShaderList { get; set; } // const char**
        public ShaderDescriptorBinding* DescriptorBindingsList { get; set; }
        public ShaderStruct* ShaderStructList { get; set; }
        public VkVertexInputBindingDescription* VertexInputBindingList { get; set; }
        public VkVertexInputAttributeDescription* VertexInputAttributeList { get; set; }
        public ShaderVariable* ShaderOutputList { get; set; }
        public ShaderPushConstant* PushConstantList { get; set; }

        public string[] GetShaderList()
        {
            if (ShaderList == IntPtr.Zero || ShaderCount == 0)
                return Array.Empty<string>();

            string[] result = new string[ShaderCount];
            for (size_t i = 0; i < ShaderCount; i++)
            {
                IntPtr stringPtr = Marshal.ReadIntPtr(ShaderList, (int)(i * IntPtr.Size));
                result[i] = stringPtr != IntPtr.Zero ? Marshal.PtrToStringAnsi(stringPtr) : null;
            }
            return result;
        }
    }
}
