using Silk.NET.Vulkan;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using System.Xml.Linq;
using Vulkan;
using VulkanGameEngineLevelEditor.GameEngine.Systems;

namespace VulkanGameEngineLevelEditor.GameEngine.Structs
{
    [StructLayout(LayoutKind.Sequential, Pack = 8)]
    public unsafe struct ShaderPushConstant
    {
        public string PushConstantName; 
        public nuint PushConstantSize;
        public VkShaderStageFlagBits ShaderStageFlags;
        public List<ShaderVariable> PushConstantVariableList;
        public void* PushConstantBuffer;
        public bool GlobalPushContant;

        public ShaderPushConstant(ShaderPushConstantDLL dll)
        {
            List<ShaderVariable> shaderVariableList = new List<ShaderVariable>();
            for (nuint x = 0; x < dll.PushConstantVariableListCount; x++)
            {
                var pushConstantVariable = dll.PushConstantVariableList[x];
                shaderVariableList.Add(new ShaderVariable(pushConstantVariable));
            }

            PushConstantName = Marshal.PtrToStringAnsi(dll.PushConstantName);
            PushConstantSize = dll.PushConstantSize;
            ShaderStageFlags = dll.ShaderStageFlags;
            PushConstantVariableList = shaderVariableList;
            PushConstantBuffer = dll.PushConstantBuffer;

            MemorySystem.RemovePtrBuffer(dll.PushConstantName);
        }

        public ShaderPushConstant(ShaderPushConstantDLL* dllPtr)
        {
            List<ShaderVariable> shaderVariableList = new List<ShaderVariable>();
            for (nuint x = 0; x < dllPtr->PushConstantVariableListCount; x++)
            {
                var pushConstantVariable = dllPtr->PushConstantVariableList[x];
                shaderVariableList.Add(new ShaderVariable(pushConstantVariable));
            }

            PushConstantName = Marshal.PtrToStringAnsi(dllPtr->PushConstantName);
            PushConstantSize = dllPtr->PushConstantSize;
            ShaderStageFlags = dllPtr->ShaderStageFlags;
            PushConstantVariableList = shaderVariableList;
            PushConstantBuffer = dllPtr->PushConstantBuffer;

            MemorySystem.RemovePtrBuffer(dllPtr->PushConstantName);
        }
    }

    [StructLayout(LayoutKind.Sequential, Pack = 8)]
    public unsafe struct ShaderPushConstantDLL
    {
        public IntPtr PushConstantName;
        public nuint PushConstantSize;
        public nuint PushConstantVariableListCount;
        public ShaderVariableDLL* PushConstantVariableList;
        public void* PushConstantBuffer;
        public VkShaderStageFlagBits ShaderStageFlags;
        public bool GlobalPushContant;

        public ShaderPushConstantDLL(ShaderPushConstant shaderStruct)
        {
            ListPtr<ShaderVariableDLL> shaderVariableList = new ListPtr<ShaderVariableDLL>();
            foreach (var shaderVar in shaderStruct.PushConstantVariableList)
            {
                shaderVariableList.Add(new ShaderVariableDLL(shaderVar));
            }

            PushConstantName = Marshal.StringToHGlobalAnsi(shaderStruct.PushConstantName);
            PushConstantSize = shaderStruct.PushConstantSize;
            PushConstantVariableListCount = (nuint)shaderVariableList.Count;
            PushConstantVariableList = shaderVariableList.Ptr;
            PushConstantBuffer = shaderStruct.PushConstantBuffer;
            ShaderStageFlags = shaderStruct.ShaderStageFlags;
            GlobalPushContant = shaderStruct.GlobalPushContant;
        }
    }
}
