using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VulkanGameEngineLevelEditor.LevelEditor.Attributes
{
    [AttributeUsage(AttributeTargets.Property)]
    public class RangeAttribute : Attribute
    {
        public float Min { get; }
        public float Max { get; }

        public RangeAttribute(float min, float max)
        {
            Min = min;
            Max = max;
        }
    }
}
