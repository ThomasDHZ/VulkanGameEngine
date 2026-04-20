using GlmSharp;
using System;
using System.Drawing;
using System.Reflection;
using System.Windows.Forms;
using VulkanGameEngineLevelEditor.LevelEditor.ControlSubForms;
using VulkanGameEngineLevelEditor.LevelEditor.EditorEnhancements;

public class TypeOfVec3Form : PropertyEditorForm
{
    public TypeOfVec3Form(ObjectPanelView rootPanel, object obj, MemberInfo member,
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

        vec3 currentVec = GetCurrentVec3();

        void AddAxis(string label, Func<vec3, float> getter, Action<vec3, float> setter)
        {
            int row = table.RowCount++;
            table.RowStyles.Add(new RowStyle(SizeType.Absolute, 32));

            var lbl = new Label
            {
                Text = label,
                Dock = DockStyle.Fill,
                TextAlign = ContentAlignment.MiddleLeft,
                ForeColor = Color.White,
                Margin = new Padding(6, 0, 0, 0)
            };
            table.Controls.Add(lbl, 0, row);

            var num = new NumericUpDown
            {
                DecimalPlaces = 4,
                Increment = 0.1m,
                Minimum = -10000000m,
                Maximum = 10000000m,
                Value = (decimal)getter(currentVec),
                Dock = DockStyle.Fill,
                BackColor = Color.FromArgb(60, 60, 60),
                ForeColor = Color.White,
                ReadOnly = _readOnly,
                Enabled = !_readOnly
            };

            num.ValueChanged += (s, e) =>
            {
                if (_readOnly) return;

                vec3 vec = GetCurrentVec3();
                setter(vec, (float)num.Value);
                SetValue(vec);       
            };

            table.Controls.Add(num, 1, row);
        }

        AddAxis("X", v => v.x, (v, val) => v.x = val);
        AddAxis("Y", v => v.y, (v, val) => v.y = val);
        AddAxis("Z", v => v.z, (v, val) => v.z = val);

        return table;
    }

    private vec3 GetCurrentVec3()
    {
        var value = GetValue();
        return value is vec3 vec ? vec : vec3.Zero;
    }
}