using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VulkanGameEngineLevelEditor.LevelEditor.Attributes
{
    [AttributeUsage(AttributeTargets.Field | AttributeTargets.Property)]
    public class NumericUpDownLimitsAttribute : Attribute
    {
        public float Increment { get; }
        public float Minimum { get; }
        public float Maximum { get; }
        public int DecimalPlaces { get; }

        public NumericUpDownLimitsAttribute(float increment = 0.1f, float minimum = -10000000f, float maximum = 10000000f, int decimalPlaces = 4)
        {
            Increment = increment;
            Minimum = minimum;
            Maximum = maximum;
            DecimalPlaces = decimalPlaces;
        }
    }
}