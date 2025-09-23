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
        public nuint ShaderStructCount { get; set; }
        public nuint VertexInputBindingCount { get; set; }
        public nuint VertexInputAttributeListCount { get; set; }
        public nuint ShaderOutputCount { get; set; }
        public nuint PushConstantCount { get; set; }
        public IntPtr ShaderList { get; set; } // const char**
        public ShaderDescriptorBindingDLL* DescriptorBindingsList { get; set; }
        public ShaderStructDLL* ShaderStructList { get; set; }
        public VkVertexInputBindingDescription* VertexInputBindingList { get; set; }
        public VkVertexInputAttributeDescription* VertexInputAttributeList { get; set; }
        public ShaderVariableDLL* ShaderOutputList { get; set; }
        public ShaderPushConstantDLL* PushConstantList { get; set; }

        public string[] GetShaderList()
        {
            if (ShaderList == IntPtr.Zero || ShaderCount == 0)
                return Array.Empty<string>();

            string[] result = new string[ShaderCount];
            for (nuint i = 0; i < ShaderCount; i++)
            {
                IntPtr stringPtr = Marshal.ReadIntPtr(ShaderList, (int)(i * (nuint)IntPtr.Size));
                result[i] = stringPtr != IntPtr.Zero ? Marshal.PtrToStringAnsi(stringPtr) : null;
            }
            return result;
        }
    }
}
