using GlmSharp;
using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using VulkanGameEngineLevelEditor.LevelEditor.ControlSubForms;
using static System.Windows.Forms.VisualStyles.VisualStyleElement;

namespace VulkanGameEngineLevelEditor.LevelEditor.Dialog
{
    public class TypeOfVec2Form : PropertyEditorForm
    {
        public TypeOfVec2Form(object obj, PropertyInfo property, int minimumPanelSize, bool readOnly) : base(obj, property, minimumPanelSize, readOnly) { }
        public override Control CreateControl()
        {
            var vec2Value = (vec2)_property.GetValue(_obj);
            int rowIndex = 0;

            var vec2Panel = new TableLayoutPanel
            {
                Dock = DockStyle.Fill,
                AutoScroll = true,
                BackColor = Color.FromArgb(70, 70, 70),
                ColumnCount = 2,
                ColumnStyles =
                {
                    new ColumnStyle(SizeType.Percent, 50F),
                    new ColumnStyle(SizeType.Percent, 50F)
                },
                RowStyles = { new RowStyle(SizeType.AutoSize, _minimumPanelSize + BufferHeight) }
            };
            CreateBaseControl(vec2Panel);

            var xLabelPanel = AddPanel();
            var xLabel = new Label { Text = "X", Dock = DockStyle.Fill, TextAlign = ContentAlignment.MiddleLeft, ForeColor = Color.White, Margin = new Padding(5) };
            xLabelPanel.Controls.Add(xLabel);
            vec2Panel.Controls.Add(xLabelPanel, 0, rowIndex);

            var xControlPanel = AddPanel();
            var numericX = new NumericUpDown
            {
                Dock = DockStyle.Fill,
                Minimum = decimal.MinValue,
                Maximum = decimal.MaxValue,
                BackColor = Color.FromArgb(60, 60, 60),
                ForeColor = Color.White,
                Value = (decimal)vec2Value.x,
                MinimumSize = new Size(0, _minimumPanelSize),
                Margin = new Padding(5)
            };
            xControlPanel.Controls.Add(numericX);
            vec2Panel.Controls.Add(xControlPanel, 1, rowIndex);

            rowIndex++;
            vec2Panel.RowCount += 1;
            vec2Panel.RowStyles.Add(new RowStyle(SizeType.AutoSize, _minimumPanelSize + BufferHeight));

            var yLabelPanel = AddPanel();
            var yLabel = new Label { Text = "Y", Dock = DockStyle.Fill, TextAlign = ContentAlignment.MiddleLeft, ForeColor = Color.White, Margin = new Padding(5) };
            yLabelPanel.Controls.Add(yLabel);
            vec2Panel.Controls.Add(yLabelPanel, 0, rowIndex);

            var yControlPanel = AddPanel();
            var numericY = new NumericUpDown
            {
                Dock = DockStyle.Fill,
                Minimum = decimal.MinValue,
                Maximum = decimal.MaxValue,
                BackColor = Color.FromArgb(60, 60, 60),
                ForeColor = Color.White,
                Value = (decimal)vec2Value.y,
                MinimumSize = new Size(0, _minimumPanelSize),
                Margin = new Padding(5)
            };
            yControlPanel.Controls.Add(numericY);
            vec2Panel.Controls.Add(yControlPanel, 1, rowIndex);

            numericX.ValueChanged += (s, e) =>
            {
                var newX = (float)((NumericUpDown)s).Value;
                var newVec2 = new vec2(newX, vec2Value.y);
                _property.SetValue(_obj, newVec2);
          //      UpdatePropertiesList.Add(new UpdateProperty { ParentObj = _parentObject, Obj = _obj });
            };

            numericY.ValueChanged += (s, e) =>
            {
                var newY = (float)((NumericUpDown)s).Value;
                var newVec2 = new vec2(vec2Value.x, newY);
                _property.SetValue(_obj, newVec2);
            //    UpdatePropertiesList.Add(new UpdateProperty { ParentObj = _parentObject, Obj = _obj });
            };

            return null;
        }

        private Panel AddPanel()
        {
            return new Panel
            {
                Dock = DockStyle.Fill,
                BackColor = Color.FromArgb(60, 60, 60),
                Margin = new Padding(2),
                Height = RowHeight
            };
        }
    }
}
