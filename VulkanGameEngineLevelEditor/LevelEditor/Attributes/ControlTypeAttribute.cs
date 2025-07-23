using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace VulkanGameEngineLevelEditor.LevelEditor.Attributes
{
    [AttributeUsage(AttributeTargets.Property, AllowMultiple = false)]
    public class ControlTypeAttribute : Attribute
    {
        public Type ControlType { get; }
        public string[] Options { get; }

        public ControlTypeAttribute(Type controlType, params string[] options)
        {
            if (!controlType.IsSubclassOf(typeof(Control)))
                throw new ArgumentException("ControlType must inherit from Control.");
            ControlType = controlType;
            Options = options;
        }
    }

}
