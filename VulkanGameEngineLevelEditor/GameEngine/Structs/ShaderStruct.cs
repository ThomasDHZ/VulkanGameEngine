using Silk.NET.Vulkan;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using System.Xml.Linq;
using VulkanGameEngineLevelEditor.GameEngine.Systems;

namespace VulkanGameEngineLevelEditor.GameEngine.Structs
{
    public unsafe struct ShaderStruct
    {
        public string Name;
        public nuint ShaderBufferSize;
        public List<ShaderVariable> ShaderBufferVariableList;
        public int ShaderStructBufferId;
        public void* ShaderStructBuffer;

        public ShaderStruct(ShaderStructDLL* dllPtr)
        {
            List<ShaderVariable> shaderVariableList = new List<ShaderVariable>();
            for (nuint x = 0; x < dllPtr->ShaderBufferVariableCount; x++)
            {
                shaderVariableList.Add(new ShaderVariable(dllPtr->ShaderBufferVariableList[x]));
            }

            Name = Marshal.PtrToStringAnsi(dllPtr->Name) ?? string.Empty;
            ShaderBufferSize = dllPtr->ShaderBufferSize;
            ShaderBufferVariableList = shaderVariableList;
            ShaderStructBufferId = dllPtr->ShaderStructBufferId;
            ShaderStructBuffer = dllPtr->ShaderStructBuffer;
            MemorySystem.RemovePtrBuffer(dllPtr->Name);
        }
    }

    [StructLayout(LayoutKind.Sequential, Pack = 8)]
    public unsafe struct ShaderStructDLL
    {
        public IntPtr Name;
        public nuint ShaderBufferSize;
        public nuint ShaderBufferVariableCount;
        public ShaderVariableDLL* ShaderBufferVariableList;
        public int ShaderStructBufferId;
        public void* ShaderStructBuffer;

        public ShaderStructDLL()
        {

        }

        public ShaderStructDLL(ShaderStruct shaderStruct)
        {
            ListPtr<ShaderVariableDLL> shaderVariableList = new ListPtr<ShaderVariableDLL>();
            foreach (var shaderVar in shaderStruct.ShaderBufferVariableList)
            {
                shaderVariableList.Add(new ShaderVariableDLL(shaderVar));
            }

            Name = Marshal.StringToHGlobalAnsi(shaderStruct.Name);
            ShaderBufferSize = shaderStruct.ShaderBufferSize;
            ShaderBufferVariableCount = (nuint)shaderVariableList.Count;
            ShaderBufferVariableList = shaderVariableList.Ptr;
            ShaderStructBufferId = shaderStruct.ShaderStructBufferId;
            ShaderStructBuffer = shaderStruct.ShaderStructBuffer;
        }
    }
}
