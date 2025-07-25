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
        public TypeOfStringForm(object obj, PropertyInfo property, int minimumPanelSize, bool readOnly)  : base(obj, property, minimumPanelSize, readOnly) { }

        public override Control CreateControl()
        {
            var textBox = new TextBox
            {
                Text = _property.GetValue(_obj)?.ToString() ?? "",
                TextAlign = HorizontalAlignment.Left
            };
            CreateBaseControl(textBox);

            if (!_readOnly)
            {
                textBox.TextChanged += (s, e) => _property.SetValue(_obj, ((TextBox)s).Text);
            }
            textBox.ReadOnly = _readOnly;

            return textBox;
        }
    }
}
