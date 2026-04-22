using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using VulkanGameEngineLevelEditor.LevelEditor.Attributes;
using VulkanGameEngineLevelEditor.LevelEditor.ControlSubForms;
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

//using System;
//using System.ComponentModel;
//using System.Drawing;
//using System.Reflection;
//using System.Windows.Forms;
//using VulkanGameEngineLevelEditor.LevelEditor.Attributes;
//using VulkanGameEngineLevelEditor.LevelEditor.ControlSubForms;
//using VulkanGameEngineLevelEditor.LevelEditor.EditorEnhancements;

//public class TypeOfFloat : PropertyEditorForm
//{
//    private readonly DynamicComponentWrapper? _wrapper;
//    private readonly MemberInfo _member;

//    public TypeOfFloat(ObjectPanelView rootPanel, object obj, MemberInfo member, int minimumPanelSize, bool readOnly)
//        : base(rootPanel, obj, member, minimumPanelSize, readOnly)
//    {
//        _member = member;
//        _wrapper = obj as DynamicComponentWrapper;
//    }

//    public override Control CreateControl()
//    {
//        var table = new TableLayoutPanel
//        {
//            Dock = DockStyle.Fill,
//            AutoSize = true,
//            ColumnCount = 2,
//            RowCount = 0,
//            BackColor = Color.FromArgb(70, 70, 70),
//            Padding = new Padding(4)
//        };
//        table.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 25F));
//        table.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 75F));

//        float currentValue = GetCurrentFloat();
//        var memberLimits = _member.GetCustomAttribute<NumericUpDownLimitsAttribute>();
//        var num = new NumericUpDown
//        {
//            DecimalPlaces = memberLimits != null ? memberLimits.DecimalPlaces : 4,
//            Increment = memberLimits != null ? Convert.ToDecimal(memberLimits.Increment) : 0.1m,
//            Minimum = memberLimits != null ? Convert.ToDecimal(memberLimits.Minimum) : -10000000m,
//            Maximum = memberLimits != null ? Convert.ToDecimal(memberLimits.Maximum) : 10000000m,
//            Value = (decimal)Math.Clamp(currentValue, memberLimits != null ? memberLimits.Minimum : -10000000f, memberLimits != null ? memberLimits.Maximum : 10000000f),
//            Dock = DockStyle.Fill,
//            BackColor = Color.FromArgb(60, 60, 60),
//            ForeColor = Color.White,
//            ReadOnly = _readOnly,
//            Enabled = !_readOnly
//        };

//        num.ValueChanged += (s, e) =>
//        {
//            if (_readOnly) return;
//            SetValue((float)num.Value);
//        };

//        num.TextChanged += (s, e) =>
//        {
//            if (_readOnly) return;
//            if (!float.TryParse(num.Text, out float parsed)) return;
//            SetValue(parsed);
//        };

//        var lbl = new Label
//        {
//            Text = _member.GetCustomAttribute<DisplayNameAttribute>()?.DisplayName ?? _member.Name,
//            Dock = DockStyle.Fill,
//            TextAlign = ContentAlignment.MiddleLeft,
//            ForeColor = Color.White,
//            Margin = new Padding(6, 0, 0, 0)
//        };

//        table.Controls.Add(lbl, 0, 0);
//        table.Controls.Add(num, 1, 0);

//        return table;
//    }

//    private float GetCurrentFloat()
//    {
//        if (_wrapper == null) return 0f;

//        var value = _wrapper.GetMemberValue(_member);
//        return value is float f ? f : 0f;
//    }
//}