using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.CompilerServices;

namespace VulkanGameEngineLevelEditor.GameEngine.Structs
{
    public unsafe struct MemoryLeakPtr
    {
        public void* PtrAddress;
        public size_t PtrElements;
        public bool isArray;
        public string DanglingPtrMessage { get; set; } = string.Empty;
        public string File { get; set; } = string.Empty;
        public string Line { get; set; } = string.Empty;
        public string Function { get; set; } = string.Empty;
        public string Notes { get; set; } = string.Empty;

        public MemoryLeakPtr()
        {
        }
    };
}
