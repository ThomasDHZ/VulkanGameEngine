using AutoMapper.Execution;
using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using VulkanGameEngineLevelEditor.LevelEditor.ControlSubForms;

namespace VulkanGameEngineLevelEditor.LevelEditor.Dialog
{
    public class TypeOfBool : PropertyEditorForm
    {
        public TypeOfBool(object obj, MemberInfo member, int minimumPanelSize, bool readOnly) : base(obj, member, minimumPanelSize, readOnly) { }
        public override Control CreateControl()
        {
            bool value = (bool)GetValue();
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
                    SetValue(((CheckBox)s).Checked);
                }
                catch (Exception ex)
                {
                    Console.WriteLine($"Error setting value: {ex.Message}");
                }
            };
            CreateBaseControl(checkBox);
            return checkBox;
        }
    }
}
