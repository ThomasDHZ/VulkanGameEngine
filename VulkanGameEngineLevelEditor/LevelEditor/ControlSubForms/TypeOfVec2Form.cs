using GlmSharp;
using System;
using System.Drawing;
using System.Reflection;
using System.Windows.Forms;
using VulkanGameEngineLevelEditor.LevelEditor.ControlSubForms;
using VulkanGameEngineLevelEditor.LevelEditor.EditorEnhancements;

public class TypeOfVec2Form : PropertyEditorForm
{
    private const int RowHeight = 32;

    public TypeOfVec2Form(ObjectPanelView rootPanel, object obj, MemberInfo member,
                          int minimumPanelSize, bool readOnly)
        : base(rootPanel, obj, member, minimumPanelSize, readOnly) { }

    public override Control CreateControl()
    {
        var table = new TableLayoutPanel
        {
            Dock = DockStyle.Fill,
            AutoSize = true,
            ColumnCount = 2,
            RowCount = 0,
            BackColor = Color.FromArgb(70, 70, 70),
            Padding = new Padding(4)
        };
        table.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 25F));
        table.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 75F));

        vec2 currentVec = GetCurrentVec2();

        void AddAxis(string label, Func<vec2, float> getter, Action<vec2, float> setter)
        {
            int row = table.RowCount++;
            table.RowStyles.Add(new RowStyle(SizeType.Absolute, RowHeight));

            // Label
            var lbl = new Label
            {
                Text = label,
                Dock = DockStyle.Fill,
                TextAlign = ContentAlignment.MiddleLeft,
                ForeColor = Color.White,
                Margin = new Padding(6, 0, 0, 0)
            };
            table.Controls.Add(lbl, 0, row);

            // NumericUpDown
            var num = new NumericUpDown
            {
                Value = (decimal)getter(currentVec),
                DecimalPlaces = 4,
                Increment = 0.1m,
                Minimum = decimal.MinValue,
                Maximum = decimal.MaxValue,
                Dock = DockStyle.Fill,
                BackColor = Color.FromArgb(60, 60, 60),
                ForeColor = Color.White,
                ReadOnly = _readOnly,
                Enabled = !_readOnly
            };

            num.ValueChanged += (s, e) =>
            {
                vec2 vec = GetCurrentVec2();
                setter(vec, (float)num.Value);
                SetValue(vec);                  
            };

            table.Controls.Add(num, 1, row);
        }

        AddAxis("X", v => v.x, (v, val) => v.x = val);
        AddAxis("Y", v => v.y, (v, val) => v.y = val);

        return table;
    }

    private vec2 GetCurrentVec2()
    {
        var value = GetValue();                
        if (value is vec2 vec) return vec;
        if (value is float f) return new vec2(f, 0f);
        return vec2.Zero;
    }
}