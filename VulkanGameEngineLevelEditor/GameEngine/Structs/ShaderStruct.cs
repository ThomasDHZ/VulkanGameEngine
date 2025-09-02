using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VulkanGameEngineLevelEditor.GameEngine.Structs
{
    public unsafe struct ShaderStruct
    {
        public fixed char Name[256];
        public size_t ShaderBufferSize { get; set; } = 0;
        public size_t ShaderBufferVariableListCount { get; set; } = 0;
        public ShaderVariable* ShaderBufferVariableList { get; set; } = null;
        public int ShaderStructBufferId { get; set; } = 0;
        public void* ShaderStructBuffer { get; set; } = null;

        public ShaderStruct()
        {
        }
    }
}
