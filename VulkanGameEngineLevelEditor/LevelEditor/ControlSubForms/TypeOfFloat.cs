using System;
using System.Drawing;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Windows.Forms;
using VulkanGameEngineLevelEditor.LevelEditor.Attributes;
using VulkanGameEngineLevelEditor.LevelEditor.ControlSubForms;
using VulkanGameEngineLevelEditor.LevelEditor.EditorEnhancements;

public unsafe class TypeOfFloat : PropertyEditorForm
{
    private readonly DynamicComponentWrapper? _wrapper;
    private readonly object? _targetObject;
    private readonly MemberInfo _member;
    private readonly IntPtr? _nativePtr;

    public TypeOfFloat(ObjectPanelView rootPanel, object obj, MemberInfo member, int minimumPanelSize, bool readOnly, IntPtr? nativePtr = null) : base(rootPanel, obj, member, minimumPanelSize, readOnly)
    {
        _member = member;
        _wrapper = obj as DynamicComponentWrapper;
        _targetObject = obj;
        _nativePtr = nativePtr;
    }

    public override Control CreateControl()
    {
        var table = new TableLayoutPanel
        {
            Dock = DockStyle.Fill,
            AutoSize = true,
            ColumnCount = 2,
            RowCount = 1,
            BackColor = Color.FromArgb(70, 70, 70),
            Padding = new Padding(4)
        };
        table.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 25F));
        table.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 75F));

        float currentValue = GetCurrentFloat();

        var memberLimits = _member.GetCustomAttribute<NumericUpDownLimitsAttribute>();

        var num = new NumericUpDown
        {
            DecimalPlaces = memberLimits != null ? memberLimits.DecimalPlaces : 4,
            Increment = memberLimits != null ? Convert.ToDecimal(memberLimits.Increment) : 0.1m,
            Minimum = memberLimits != null ? Convert.ToDecimal(memberLimits.Minimum) : -10000000m,
            Maximum = memberLimits != null ? Convert.ToDecimal(memberLimits.Maximum) : 10000000m,
            Value = (decimal)Math.Clamp(currentValue, memberLimits?.Minimum ?? -10000000f, memberLimits?.Maximum ?? 10000000f),
            Dock = DockStyle.Fill,
            BackColor = Color.FromArgb(60, 60, 60),
            ForeColor = Color.White,
            ReadOnly = _readOnly,
            Enabled = !_readOnly
        };

        num.ValueChanged += (s, e) =>
        {
            if (_readOnly) return;
            SetFloatDirect((float)num.Value);
        };

        num.TextChanged += (s, e) =>
        {
            if (_readOnly) return;
            if (float.TryParse(num.Text, out float parsed)) SetFloatDirect(parsed);
        };

        var lbl = new Label
        {
            Text = "Value",
            Dock = DockStyle.Fill,
            TextAlign = ContentAlignment.MiddleLeft,
            ForeColor = Color.White,
            Margin = new Padding(6, 0, 0, 0)
        };

        table.Controls.Add(lbl, 0, 0);
        table.Controls.Add(num, 1, 0);

        return table;
    }

    private float GetCurrentFloat()
    {
        if (_wrapper != null)
        {
            var value = _wrapper.GetMemberValue(_member);
            return value is float f ? f : 0f;
        }

        if (_targetObject != null)
        {
            if (_member is FieldInfo fi) return (float?)fi.GetValue(_targetObject) ?? 0f;
            if (_member is PropertyInfo pi && pi.CanRead) return (float?)pi.GetValue(_targetObject) ?? 0f;
        }

        return 0f;
    }

    private void SetFloatDirect(float newValue)
    {
        if (_nativePtr.HasValue && _nativePtr.Value != IntPtr.Zero && _member is FieldInfo fieldInfo)
        {
            try
            {
                int offset = Marshal.OffsetOf(_targetObject!.GetType(), fieldInfo.Name).ToInt32();
                ref float target = ref Unsafe.AsRef<float>((byte*)_nativePtr.Value + offset);
                target = newValue;
                return;
            }
            catch (Exception ex)
            {
                Console.WriteLine($"[Float Native Linked FAILED] {ex.Message}");
            }
        }

        if (_wrapper != null && _member is FieldInfo fi)
        {
            try
            {
                int offset = Marshal.OffsetOf(_wrapper.ComponentStructType, fi.Name).ToInt32();
                ref float target = ref Unsafe.AsRef<float>((byte*)_wrapper.ComponentPtr.ToPointer() + offset);

                target = newValue;
                return;
            }
            catch { }
        }

        if (_targetObject != null)
        {
            try
            {
                if (_member is FieldInfo fi2) fi2.SetValue(_targetObject, newValue);
                else if (_member is PropertyInfo pi && pi.CanWrite) pi.SetValue(_targetObject, newValue);
            }
            catch (Exception ex)
            {
                Console.WriteLine($"[SetFloatDirect fallback] Error: {ex.Message}");
            }
        }
    }
}