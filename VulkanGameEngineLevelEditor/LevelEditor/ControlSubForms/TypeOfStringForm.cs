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
using VulkanGameEngineLevelEditor.LevelEditor.EditorEnhancements;

namespace VulkanGameEngineLevelEditor.LevelEditor.Dialog
{
    public class TypeOfStringForm : PropertyEditorForm
    {
        public TypeOfStringForm(ObjectPanelView rootPanel, object obj, MemberInfo member, int minimumPanelSize, bool readOnly) : base(rootPanel, obj, member, minimumPanelSize, readOnly) { }
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
