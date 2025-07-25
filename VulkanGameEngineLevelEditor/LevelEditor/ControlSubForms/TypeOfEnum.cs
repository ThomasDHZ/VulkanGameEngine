using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using static System.Windows.Forms.VisualStyles.VisualStyleElement;

namespace VulkanGameEngineLevelEditor.LevelEditor.ControlSubForms
{
    public class TypeOfEnum : PropertyEditorForm
    {
        private System.Windows.Forms.ComboBox comboBox1;

        public TypeOfEnum(object obj, PropertyInfo property, int minimumPanelSize, bool readOnly) : base(obj, property, minimumPanelSize, readOnly) { }
        public override Control CreateControl()
        {
            comboBox1 = new System.Windows.Forms.ComboBox();
            // 
            // comboBox1
            // 
            comboBox1.BackColor = System.Drawing.Color.FromArgb(40, 40, 40);
            comboBox1.Dock = System.Windows.Forms.DockStyle.Fill;
            comboBox1.FormattingEnabled = true;
            comboBox1.Location = new System.Drawing.Point(0, 0);
            comboBox1.Name = "comboBox1";
            comboBox1.Size = new System.Drawing.Size(233, 33);
            comboBox1.TabIndex = 0;

            Type enumType = null;
            if (_property.PropertyType.BaseType is Type)
            {
                enumType = _property.PropertyType;
            }
            else
            {
                enumType = _property.PropertyType.BaseType.GetType();
            }

            if (enumType.IsEnum)
            {
                Array values = Enum.GetValues(enumType);

                var filteredValues = new List<object>();
                foreach (var val in values)
                {
                    filteredValues.Add(val);
                }

                comboBox1.DataSource = filteredValues;
            }
            else
            {
                throw new InvalidOperationException("Object is not an enum type");
            }

            CreateBaseControl(comboBox1);
            return comboBox1;
        }
    }
}