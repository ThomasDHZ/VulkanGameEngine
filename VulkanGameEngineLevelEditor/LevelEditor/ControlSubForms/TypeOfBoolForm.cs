using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace VulkanGameEngineLevelEditor.LevelEditor.Dialog
{
    public class TypeOfBool : PropertyEditorForm
    {
        public TypeOfBool(object obj, PropertyInfo property, int minimumPanelSize, bool readOnly) : base(obj, property, minimumPanelSize, readOnly) { }
        public override Control CreateControl()
        {
            bool value = (bool)_property.GetValue(_obj);
            var checkBox = new CheckBox
            {
                Dock = DockStyle.Fill,
                Checked = value,
                MinimumSize = new Size(0, _minimumPanelSize)
            };
            checkBox.CheckedChanged += (s, e) =>
            {
                try
                {
                    _property.SetValue(_obj, ((CheckBox)s).Checked);
                }
                catch (Exception ex)
                {
                    Console.WriteLine($"Error setting {_property.Name}: {ex.Message}");
                }
            };
            CreateBaseControl(checkBox);
            return checkBox;
        }
    }
}
