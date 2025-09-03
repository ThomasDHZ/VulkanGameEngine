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
        public fixed char Name[256];
        public nuint ShaderBufferSize { get; set; }
        public nuint ShaderBufferVariableListCount { get; set; }
        public ShaderVariable* ShaderBufferVariableList { get; set; }
        public int ShaderStructBufferId { get; set; }
        public void* ShaderStructBuffer { get; set; }

        public ShaderStruct()
        {
            ShaderBufferSize = 0;
            ShaderBufferVariableListCount = 0;
            ShaderBufferVariableList = null;
            ShaderStructBufferId = 0;
            ShaderStructBuffer = null;
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
                ptr[length] = '\0';
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
