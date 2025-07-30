using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using VulkanGameEngineLevelEditor.LevelEditor.ControlSubForms;
using static System.Windows.Forms.VisualStyles.VisualStyleElement.Button;

namespace VulkanGameEngineLevelEditor.LevelEditor.Dialog
{
    public class TypeOfGuidForm : PropertyEditorForm
    {
        public TypeOfGuidForm(object obj, MemberInfo member, int minimumPanelSize, bool readOnly) : base(obj, member, minimumPanelSize, readOnly) { }
        public override Control CreateControl()
        {
            string guid = ((Guid)GetValue()).ToString();
            var textBox = new TextBox
            {
                Dock = DockStyle.Fill,
                Text = guid ?? "",
                TextAlign = HorizontalAlignment.Left,
                BackColor = Color.FromArgb(60, 60, 60),
                ForeColor = Color.White,
                ReadOnly = true,
                MinimumSize = new Size(0, _minimumPanelSize)
            };
            CreateBaseControl(textBox);
            return textBox;
        }
    }
}
