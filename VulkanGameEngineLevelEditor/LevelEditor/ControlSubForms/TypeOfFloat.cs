using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using VulkanGameEngineLevelEditor.LevelEditor.EditorEnhancements;

namespace VulkanGameEngineLevelEditor.LevelEditor.ControlSubForms
{
    public class TypeOfFloat : PropertyEditorForm
    {
        public TypeOfFloat(ObjectPanelView rootPanel, object obj, MemberInfo member, int minimumPanelSize, bool readOnly) : base(rootPanel, obj, member, minimumPanelSize, readOnly) { }

        public override Control CreateControl()
        {
            float value = (float)GetValue();
            if (_readOnly)
            {
                var labelDisplay = new Label
                {
                    Dock = DockStyle.Fill,
                    Text = value.ToString("F2") ?? "N/A",
                    TextAlign = ContentAlignment.MiddleRight,
                    BackColor = Color.FromArgb(60, 60, 60),
                    BorderStyle = BorderStyle.FixedSingle,
                    ForeColor = Color.White,
                    MinimumSize = new Size(0, _minimumPanelSize)
                };
                CreateBaseControl(labelDisplay);
                return labelDisplay;
            }
            else
            {
                var textBox = new TextBox
                {
                    Text = GetValue()?.ToString() ?? "",
                    TextAlign = HorizontalAlignment.Left
                };
                if (!_readOnly)
                {
                    textBox.TextChanged += (s, e) => SetValue(((TextBox)s).Text);
                }
                textBox.ReadOnly = _readOnly;
               
                CreateBaseControl(textBox);
                return textBox;
            }
        }
    }
}
