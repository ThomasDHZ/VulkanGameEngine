using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VulkanEngineCS.Models
{
    public struct GameObjectVariable
    {
        public string VariableName { get; set; } = string.Empty;
        public object Value { get; set; }
        public bool ConstVariable { get; set; } = false;
        public uint VariableByteSize { get; set; } = uint.MaxValue;
        public GameObjectMemberType MemberTypeEnum { get; set; } = GameObjectMemberType.GameObjectVarUnknown;
        public GameObjectVariable()
        {
        }
    }
}
