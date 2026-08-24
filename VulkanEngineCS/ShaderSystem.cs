using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using System.Xml.Linq;
using System.Xml.Serialization;
using VulkanCS;
using VulkanEngineCoreCS;
using VulkanEngineCoreCS.Models;

namespace VulkanEngineCS
{
    public static unsafe class ShaderSystem
    {
        public static VulkanShader LoadShader([MarshalAs(UnmanagedType.LPStr)] string shaderPath)
        {
            IntPtr pAnsi = Marshal.StringToHGlobalAnsi(shaderPath);
            VulkanShaderDLL dll = DLLSystem.CallDLLFunc(() => ShaderSystem_LoadShader(shaderPath));

            string? pushConstantName = null;
            var pushConstantVariables = new List<ShaderVariable>();
            if (dll.PushConstant.PushConstantVariableList != null && dll.PushConstant.PushConstantVariableCount > 0)
            {
                for (int x = 0; x < (int)dll.PushConstant.PushConstantVariableCount; x++)
                {
                    ref ShaderVariableDLL src = ref dll.PushConstant.PushConstantVariableList[x];

                    pushConstantVariables.Add(new ShaderVariable
                    {
                        Name = Marshal.PtrToStringAnsi(src.Name).ToString(),
                        Size = src.Size,
                        ByteAlignment = src.ByteAlignment,
                        MemberTypeEnum = src.MemberTypeEnum,
                        ConstVariable = src.ConstVariable,
                        Value = null
                    });

                   // MemorySystem.DeletePtr(src.Name);
                }

                pushConstantName = dll.PushConstant.PushConstantName;
                MemorySystem.DeletePtr(dll.PushConstant.PushConstantName);
               // MemorySystem.DeletePtr(dll.PushConstant.PushConstantVariableList);
            }

            var pushConstant = new ShaderPushConstant
            {
                PushConstantName = pushConstantName,
                PushConstantSize = dll.PushConstant.PushConstantSize,
                ShaderStageFlags = dll.PushConstant.ShaderStageFlags,
                PushConstantVariableList = pushConstantVariables,
                GlobalPushConstant = dll.PushConstant.GlobalPushConstant
            };

            var descriptorBindings = new List<ShaderDescriptorBinding>();

            if (dll.DescriptorBindingList != null && dll.DescriptorBindingCount > 0)
            {
                for (int x = 0; x < (int)dll.DescriptorBindingCount; x++)
                {
                    ref ShaderDescriptorBindingDLL src = ref dll.DescriptorBindingList[x];

                    //var imageInfos = new List<VkDescriptorImageInfo>();
                    //if (src.DescriptorImageInfo != null && src.DescriptorImageInfoCount > 0)
                    //{
                    //    for (int y = 0; y < (int)src.DescriptorImageInfoCount; y++)
                    //    {
                    //        imageInfos.Add(src.DescriptorImageInfo[y]);
                    //    }
                    //    MemorySystem.DeletePtr(src.DescriptorImageInfo);
                    //}

                    //var bufferInfos = new List<VkDescriptorBufferInfo>();
                    //if (src.DescriptorBufferInfo != null && src.DescriptorBufferInfoCount > 0)
                    //{
                    //    for (int y = 0; y < (int)src.DescriptorBufferInfoCount; y++)
                    //    {
                    //        bufferInfos.Add(src.DescriptorBufferInfo[y]);
                    //    }
                    //    MemorySystem.DeletePtr(src.DescriptorBufferInfo);
                    //}

                    descriptorBindings.Add(new ShaderDescriptorBinding
                    {
                        Name = Marshal.PtrToStringAnsi(src.Name).ToString(),
                        DescriptorSet = src.DescriptorSet,
                        Binding = src.Binding,
                        ShaderStageFlags = src.ShaderStageFlags,
                        DescriptorBindingType = src.DescriptorBindingType,
                        DescripterType = src.DescripterType,
                       // DescriptorImageInfo = imageInfos,
                       // DescriptorBufferInfo = bufferInfos
                    });

                   // MemorySystem.DeletePtr(src.Name);
                }
             //   MemorySystem.DeletePtr(dll.DescriptorBindingList);
            }

            var vertexInputBindings = new List<VkVertexInputBindingDescription>();
            if (dll.VertexInputBindingList != null && dll.VertexInputBindingCount > 0)
            {
                for (int x = 0; x < (int)dll.VertexInputBindingCount; x++)
                {
                    vertexInputBindings.Add(dll.VertexInputBindingList[x]);
                }
                MemorySystem.DeletePtr(dll.VertexInputBindingList);
            }

            var inputAttributes = new List<VkVertexInputAttributeDescription>();
            if (dll.InputVertexAttributeList != null && dll.InputVertexAttributeCount > 0)
            {
                for (int x = 0; x < (int)dll.InputVertexAttributeCount; x++)
                {
                    inputAttributes.Add(dll.InputVertexAttributeList[x]);
                }
                MemorySystem.DeletePtr(dll.InputVertexAttributeList);
            }

            var outputAttributes = new List<VkVertexInputAttributeDescription>();
            if (dll.OutputVertexAttributeList != null && dll.OutputVertexAttributeCount > 0)
            {
                for (int x = 0; x < (int)dll.OutputVertexAttributeCount; x++)
                {
                    outputAttributes.Add(dll.OutputVertexAttributeList[x]);
                }
                MemorySystem.DeletePtr(dll.OutputVertexAttributeList);
            }

            return new VulkanShader
            {
                ShaderModule = dll.ShaderModule,
                ShaderStages = dll.ShaderStages,
                PushConstant = pushConstant,
                InputVertexAttributeList = inputAttributes,
                OutputVertexAttributeList = outputAttributes,
                VertexInputBindingList = vertexInputBindings,
                DescriptorBindingList = descriptorBindings
            };
        }

        [DllImport("VulkanEngineInterop.dll", CallingConvention = CallingConvention.Cdecl)] private static extern VulkanShaderDLL ShaderSystem_LoadShader([MarshalAs(UnmanagedType.LPStr)] string shaderPath);
    }
}
