using System;
using System.Collections.Generic;
using System.DirectoryServices.ActiveDirectory;
using System.Drawing;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using VulkanGameEngineLevelEditor.LevelEditor.EditorEnhancements;
using static System.Windows.Forms.VisualStyles.VisualStyleElement;

namespace VulkanGameEngineLevelEditor.LevelEditor.ControlSubForms
{
    public class TypeOfEnum : PropertyEditorForm
    {
        private System.Windows.Forms.ComboBox comboBox1;

        public TypeOfEnum(ObjectPanelView rootPanel, object obj, MemberInfo member, int minimumPanelSize, bool readOnly) : base(rootPanel, obj, member, minimumPanelSize, readOnly) { }
        public override Control CreateControl()
        {
            comboBox1 = new System.Windows.Forms.ComboBox
            {
                BackColor = System.Drawing.Color.FromArgb(40, 40, 40),
                ForeColor = System.Drawing.Color.White,
                Dock = System.Windows.Forms.DockStyle.Fill,
                FormattingEnabled = true,
                Location = new System.Drawing.Point(0, 0),
                Name = "comboBox1",
                Size = new System.Drawing.Size(233, 33),
                TabIndex = 0,
                DropDownStyle = ComboBoxStyle.DropDownList,
                FlatStyle = FlatStyle.Flat
            };

            Type enumType = null;
            object currentValue = GetValue();
            if (_member is PropertyInfo propType)
            {
                if (propType.PropertyType.IsEnum)
                {
                    enumType = propType.PropertyType;
                }
                else if (propType.PropertyType.BaseType != null && propType.PropertyType.BaseType.IsEnum)
                {
                    enumType = propType.PropertyType.BaseType;
                }
            }
            else if (_member is FieldInfo fieldType)
            {
                if (fieldType.FieldType.IsEnum)
                {
                    enumType = fieldType.FieldType;
                }
                else if (fieldType.FieldType.BaseType != null && fieldType.FieldType.BaseType.IsEnum)
                {
                    enumType = fieldType.FieldType.BaseType;
                }
            }

            if (enumType != null && enumType.IsEnum)
            {
                Array values = Enum.GetValues(enumType);
                var filteredValues = new List<object>();
                foreach (var val in values)
                {
                    filteredValues.Add(val);
                }

                comboBox1.Items.Clear();
                comboBox1.Items.AddRange(filteredValues.ToArray());
                comboBox1.SelectedIndex = -1;
                if (currentValue != null && !_readOnly)
                {
                    try
                    {
                        object enumVal = Enum.ToObject(enumType, currentValue);
                        int index = filteredValues.IndexOf(enumVal);
                        if (index >= 0 && index < filteredValues.Count)
                        {
                            comboBox1.SelectedIndex = index;
                        }
                        else
                        {
                            comboBox1.SelectedIndex = -1;
                        }
                    }
                    catch (Exception ex)
                    {
                        comboBox1.SelectedIndex = -1;
                    }
                }
                else
                {
                    comboBox1.SelectedIndex = -1;
                }
            }
            else
            {
                throw new InvalidOperationException("Object is not an enum type");
            }

            CreateBaseControl(comboBox1);
            comboBox1.Parent?.Focus();
            return comboBox1;
        }
    }
}