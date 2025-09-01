using System;
using System.Collections.Generic;
using System.Linq;
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

    public unsafe struct ShaderVariable
    {
        public string Name { get; set; } = string.Empty;
        public size_t Size { get; set; } = 0;
        public size_t ByteAlignment { get; set; } = 0;
        public void* Value { get; set; } = null;
        public ShaderMemberType MemberTypeEnum { get; set; } = ShaderMemberType.shaderUnknown;

        public ShaderVariable()
        {
        }
    }
}
