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
        public size_t ShaderCount;
        public size_t DescriptorBindingCount;
        public size_t VertexInputBindingCount;
        public size_t VertexInputAttributeListCount;
        public size_t PushConstantCount;
        public ShaderDescriptorBinding* DescriptorBindingsList;
        public VkVertexInputBindingDescription* VertexInputBindingList;
        public VkVertexInputAttributeDescription* VertexInputAttributeList;
        public ShaderPushConstant* PushConstantList;
        public IntPtr ShaderList; 

        public string[] GetShaderList()
        {
            if (ShaderList == IntPtr.Zero || ShaderCount == 0)
                return Array.Empty<string>();

            string[] result = new string[ShaderCount];
            for (size_t i = 0; i < ShaderCount; i++)
            {
                IntPtr stringPtr = Marshal.ReadIntPtr(ShaderList, (int)i * IntPtr.Size);
                result[(int)i] = Marshal.PtrToStringAnsi(stringPtr) ?? string.Empty;
            }
            return result;
        }
    }
}
