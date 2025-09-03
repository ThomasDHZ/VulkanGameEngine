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
        public fixed char Name[256];
        public nuint Size { get; set; }
        public nuint ByteAlignment { get; set; }
        public void* Value { get; set; }
        public ShaderMemberType MemberTypeEnum { get; set; }

        public ShaderVariable()
        {
            Size = 0;
            ByteAlignment = 0;
            Value = null;
            MemberTypeEnum = ShaderMemberType.shaderUnknown;
            // Initialize Name to empty null-terminated string
            fixed (char* ptr = Name)
            {
                ptr[0] = '\0';
            }
        }

        public void SetName(string name)
        {
            if (string.IsNullOrEmpty(name))
            {
                fixed (char* ptr = Name)
                {
                    ptr[0] = '\0';
                }
                return;
            }

            fixed (char* ptr = Name)
            {
                int length = Math.Min(name.Length, 255);
                for (int i = 0; i < length; i++)
                {
                    ptr[i] = name[i];
                }
                ptr[length] = '\0'; // Null-terminate
            }
        }

        public string GetName()
        {
            fixed (char* ptr = Name)
            {
                return new string(ptr);
            }
        }
    }
}
