using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace VulkanGameEngineLevelEditor.GameEngine.Structs
{
    [StructLayout(LayoutKind.Sequential, Pack = 8)]
    public unsafe struct ShaderStruct
    {
        public IntPtr Name;
        public nuint ShaderBufferSize;
        public nuint ShaderBufferVariableListCount;
        public ShaderVariable* ShaderBufferVariableList;
        public int ShaderStructBufferId;
        public void* ShaderStructBuffer;

        public string GetName() => Marshal.PtrToStringAnsi(Name) ?? string.Empty;
    }
}
