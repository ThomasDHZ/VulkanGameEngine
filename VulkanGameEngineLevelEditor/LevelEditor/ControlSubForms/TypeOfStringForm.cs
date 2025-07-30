using Newtonsoft.Json.Linq;
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
    public class TypeOfStringForm : PropertyEditorForm
    {
        public TypeOfStringForm(object obj, MemberInfo member, int minimumPanelSize, bool readOnly) : base(obj, member, minimumPanelSize, readOnly) { }

        public override Control CreateControl()
        {
            var textBox = new TextBox
            {
                Text = GetValue()?.ToString() ?? "",
                TextAlign = HorizontalAlignment.Left
            };
            CreateBaseControl(textBox);

            if (!_readOnly)
            {
                textBox.TextChanged += (s, e) => SetValue(((TextBox)s).Text);
            }
            textBox.ReadOnly = _readOnly;

            return textBox;
        }
    }
}
