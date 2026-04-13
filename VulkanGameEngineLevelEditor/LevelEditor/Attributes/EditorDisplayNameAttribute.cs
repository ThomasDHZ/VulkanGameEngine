using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VulkanGameEngineLevelEditor.LevelEditor.Attributes
{
    [AttributeUsage(AttributeTargets.Field | AttributeTargets.Property)]
    public class EditorDisplayNameAttribute : Attribute
    {
        public string Name { get; }
        public EditorDisplayNameAttribute(string name) => Name = name;
    }
}
