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
    public enum ShaderMemberType
    {
        shaderUnknown,
        shaderInt,
        shaderUint,
        shaderFloat,
        shaderIvec2,
        shaderIvec3,
        shaderIvec4,
        shaderVec2,
        shaderVec3,
        shaderVec4,
        shaderMat2,
        shaderMat3,
        shaderMat4,
        shaderbool
    }

    [StructLayout(LayoutKind.Sequential, Pack = 8)]
    public unsafe struct ShaderVariable
    {
        public string Name;
        public nuint Size;
        public nuint ByteAlignment;
        public void* Value;
        public ShaderMemberType MemberTypeEnum;

        public ShaderVariable()
        {

        }

        public ShaderVariable(ShaderVariableDLL variableDLL)
        {

            Name = Marshal.PtrToStringAnsi(variableDLL.Name);
            ByteAlignment = variableDLL.ByteAlignment;
            MemberTypeEnum = variableDLL.MemberTypeEnum;
            Size = variableDLL.Size;
            Value = MemorySystem.AddPtrBuffer<byte>((byte)variableDLL.Size);
            MemorySystem.RemovePtrBuffer(variableDLL.Value);
        }
    }

    [StructLayout(LayoutKind.Sequential, Pack = 8)]
    public unsafe struct ShaderVariableDLL
    {
        public IntPtr Name;
        public nuint Size;
        public nuint ByteAlignment;
        public IntPtr Value;
        public ShaderMemberType MemberTypeEnum;
    }
}
