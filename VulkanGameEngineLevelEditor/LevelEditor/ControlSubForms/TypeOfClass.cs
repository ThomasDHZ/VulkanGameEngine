using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace VulkanGameEngineLevelEditor.LevelEditor.ControlSubForms
{
    public class TypeOfClass : PropertyEditorForm
    {
        public TypeOfClass(object obj, MemberInfo member, int minimumPanelSize, bool readOnly) : base(obj, member, minimumPanelSize, readOnly) { }

        public override Control CreateControl()
        {
            var labelDisplay = new Button
            {
                Name = "button1",
                Size = new System.Drawing.Size(112, 34),
                TabIndex = 0,
                Text = "Goto",
                UseVisualStyleBackColor = true
            };
            CreateBaseControl(labelDisplay);
            return labelDisplay;
        }
    }
}
