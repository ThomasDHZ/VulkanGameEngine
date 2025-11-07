using GlmSharp;
using System.Drawing;
using System.Reflection;
using System.Windows.Forms;
using VulkanGameEngineLevelEditor.LevelEditor.ControlSubForms;
using VulkanGameEngineLevelEditor.LevelEditor.EditorEnhancements;

public class TypeOfVec2Form : PropertyEditorForm
{
    private const int RowHeight = 32;    
    private const int BufferHeight = 4;  

    public TypeOfVec2Form(ObjectPanelView rootPanel, object obj, MemberInfo member,
                          int minimumPanelSize, bool readOnly)
        : base(rootPanel, obj, member, minimumPanelSize, readOnly) { }

    public override Control CreateControl()
    {
        var table = new TableLayoutPanel
        {
            Dock = DockStyle.Fill,
            AutoSize = true,
            AutoScroll = true,
            BackColor = Color.FromArgb(70, 70, 70),
            ColumnCount = 2,
            RowCount = 0,
            Padding = new Padding(0) 
        };
        table.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 30F));
        table.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 70F));

        void AddRow(string axis, float value, bool addDivider = false)
        {
            int row = table.RowCount++;
            table.RowStyles.Add(new RowStyle(SizeType.Absolute, RowHeight));

            var lblPanel = new Panel { Dock = DockStyle.Fill, Margin = new Padding(2) };
            var lbl = new Label
            {
                Text = axis,
                Dock = DockStyle.Fill,
                TextAlign = ContentAlignment.MiddleLeft,
                ForeColor = Color.White,
                Margin = new Padding(5, 0, 0, 0)
            };
            lblPanel.Controls.Add(lbl);
            table.Controls.Add(lblPanel, 0, row);

            var numPanel = new Panel { Dock = DockStyle.Fill, Margin = new Padding(2) };
            var num = new NumericUpDown
            {
                Dock = DockStyle.Fill,
                Minimum = decimal.MinValue,
                Maximum = decimal.MaxValue,
                DecimalPlaces = 6,
                Increment = 0.1m,
                Value = (decimal)value,
                BackColor = Color.FromArgb(60, 60, 60),
                ForeColor = Color.White,
                Margin = new Padding(0),
                ReadOnly = _readOnly,
                Enabled = !_readOnly,
                BorderStyle = BorderStyle.FixedSingle
            };
            numPanel.Controls.Add(num);
            table.Controls.Add(numPanel, 1, row);

            num.ValueChanged += (s, e) =>
            {
                var vec = GetCurrentVec2();
                if (axis == "X") vec.x = (float)num.Value;
                else vec.y = (float)num.Value;
                SetValue(vec);
            };

            if (addDivider)
            {
                int divRow = table.RowCount++;
                table.RowStyles.Add(new RowStyle(SizeType.Absolute, 1));
                var line = new Panel
                {
                    Dock = DockStyle.Fill,
                    BackColor = Color.FromArgb(40, 40, 40),
                    Margin = new Padding(0, 0, 0, 4)
                };
                table.Controls.Add(line, 0, divRow);
                table.SetColumnSpan(line, 2);
            }
        }

        var vec = (vec2)GetValue();

        AddRow("X", vec.x, addDivider: true); 
        AddRow("Y", vec.y, addDivider: true); 

        return table;
    }

    private vec2 GetCurrentVec2()
        => (vec2)GetValue();
}