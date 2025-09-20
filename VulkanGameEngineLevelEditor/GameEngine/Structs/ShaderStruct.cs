using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using VulkanGameEngineLevelEditor.GameEngine.Systems;

namespace VulkanGameEngineLevelEditor.GameEngine.Structs
{
    public unsafe struct ShaderStruct
    {
        public String Name;
        public nuint ShaderBufferSize = 0;
        public ListPtr<ShaderVariable> ShaderBufferVariableList;
        public int ShaderStructBufferId;
        public void* ShaderStructBuffer = null;

        public ShaderStruct()
        {
        }

        public ShaderStruct(ShaderStructDLL shaderStructDLL)
        {
            Name = shaderStructDLL.Name;
            ShaderBufferSize = shaderStructDLL.ShaderBufferSize;
            ShaderBufferVariableList = new ListPtr<ShaderVariable>(shaderStructDLL.ShaderBufferVariableList, (nint)shaderStructDLL.ShaderBufferVariableListCount);
            ShaderStructBufferId = shaderStructDLL.ShaderStructBufferId;
            ShaderStructBuffer = shaderStructDLL.ShaderStructBuffer;

            //for (nuint x = 0; x < shaderStructDLL.ShaderBufferVariableListCount; x++)
            //{
            //    MemorySystem.RemovePtrBuffer<ShaderVariable>(shaderStructDLL.ShaderBufferVariableList[x]);
            //}
        }
    };

    [StructLayout(LayoutKind.Sequential, Pack = 8)]
    public unsafe struct ShaderStructDLL
    {
        [MarshalAs(UnmanagedType.LPStr)]
        public string Name;
        public nuint ShaderBufferSize;
        public nuint ShaderBufferVariableListCount;
        public ShaderVariable* ShaderBufferVariableList;
        public int ShaderStructBufferId;
        public void* ShaderStructBuffer;
    }
}
