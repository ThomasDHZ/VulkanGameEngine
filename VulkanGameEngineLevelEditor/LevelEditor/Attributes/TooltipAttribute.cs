using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VulkanGameEngineLevelEditor.LevelEditor.Attributes
{
    [AttributeUsage(AttributeTargets.Property)]
    public class TooltipAttribute : Attribute
    {
        public string Tooltip { get; }

        public TooltipAttribute(string tooltip)
        {
            Tooltip = tooltip;
        }
    }
}
