using GlmSharp;
using System;
using System.Drawing;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Windows.Forms;
using VulkanGameEngineLevelEditor.LevelEditor.Attributes;
using VulkanGameEngineLevelEditor.LevelEditor.ControlSubForms;
using VulkanGameEngineLevelEditor.LevelEditor.EditorEnhancements;

public unsafe class TypeOfVec3 : PropertyEditorForm
{
    private const int RowHeight = 32;

    private readonly DynamicComponentWrapper? _wrapper;
    private readonly object? _targetObject;
    private readonly MemberInfo _member;
    private readonly IntPtr? _nativePtr;

    public TypeOfVec3(ObjectPanelView rootPanel, object obj, MemberInfo member, int minimumPanelSize, bool readOnly, IntPtr? nativePtr = null) : base(rootPanel, obj, member, minimumPanelSize, readOnly)  
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
            RowCount = 0,
            BackColor = Color.FromArgb(70, 70, 70),
            Padding = new Padding(4)
        };
        table.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 25F));
        table.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 75F));

        vec3 currentVec = GetCurrentVec3();
        void AddAxis(string label, Func<vec3, float> getter, int axis)
        {
            int row = table.RowCount++;
            table.RowStyles.Add(new RowStyle(SizeType.Absolute, RowHeight));

            var lbl = new Label
            {
                Text = label,
                Dock = DockStyle.Fill,
                TextAlign = ContentAlignment.MiddleLeft,
                ForeColor = Color.White,
                Margin = new Padding(6, 0, 0, 0)
            };
            table.Controls.Add(lbl, 0, row);

            float currentValue = getter(currentVec);
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
                SetAxis(axis, (float)num.Value);
            };

            num.TextChanged += (s, e) =>
            {
                if (_readOnly) return;
                if (float.TryParse(num.Text, out float parsed))
                    SetAxis(axis, parsed);
            };

            table.Controls.Add(num, 1, row);
        }

        AddAxis("X", v => v.x, 0);
        AddAxis("Y", v => v.y, 1);
        AddAxis("Z", v => v.z, 2);

        return table;
    }

    private void SetAxis(int axis, float newValue)
    {
        if (_nativePtr.HasValue && _nativePtr.Value != IntPtr.Zero && _member is FieldInfo fieldInfo)
        {
            try
            {
                int offset = Marshal.OffsetOf(_targetObject!.GetType(), fieldInfo.Name).ToInt32();
                ref vec3 vec = ref Unsafe.AsRef<vec3>((byte*)_nativePtr.Value + offset);

                switch (axis)
                {
                    case 0: vec.x = newValue; break;
                    case 1: vec.y = newValue; break;
                    case 2: vec.z = newValue; break;
                }
                return;
            }
            catch (Exception ex)
            {
                Console.WriteLine($"[Vec3 Native Linked FAILED] {ex.Message}");
            }
        }

        if (_wrapper != null && _member is FieldInfo fi)
        {
            try
            {
                int offset = Marshal.OffsetOf(_wrapper.ComponentStructType, fi.Name).ToInt32();
                ref vec3 vec = ref Unsafe.AsRef<vec3>((byte*)_wrapper.ComponentPtr.ToPointer() + offset);
                switch (axis)
                {
                    case 0: vec.x = newValue; break;
                    case 1: vec.y = newValue; break;
                    case 2: vec.z = newValue; break;
                }
                return;
            }
            catch { }
        }
    }

    private vec3 GetCurrentVec3()
    {
        if (_wrapper != null)
        {
            var value = _wrapper.GetMemberValue(_member);
            return value is vec3 vec ? vec : vec3.Zero;
        }

        if (_targetObject != null)
        {
            if (_member is FieldInfo fi) return (vec3?)fi.GetValue(_targetObject) ?? vec3.Zero;
            if (_member is PropertyInfo pi && pi.CanRead) return (vec3?)pi.GetValue(_targetObject) ?? vec3.Zero;
        }
        return vec3.Zero;
    }
}