using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

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
        [MarshalAs(UnmanagedType.LPStr)]
        public string Name;
        public nuint Size;
        public nuint ByteAlignment;
        public void* Value;
        public ShaderMemberType MemberTypeEnum;
    }
}
