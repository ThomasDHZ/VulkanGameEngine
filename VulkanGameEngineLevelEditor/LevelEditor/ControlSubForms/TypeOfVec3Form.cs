using AutoMapper.Execution;
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

public unsafe class TypeOfVec3Form : PropertyEditorForm
{
    private const int RowHeight = 32;

    private readonly DynamicComponentWrapper? _wrapper;
    private readonly MemberInfo _member;

    public TypeOfVec3Form(ObjectPanelView rootPanel, object obj, MemberInfo member, int minimumPanelSize, bool readOnly)
        : base(rootPanel, obj, member, minimumPanelSize, readOnly)
    {
        _member = member;
        _wrapper = obj as DynamicComponentWrapper;
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

        if (_wrapper == null || _wrapper.ComponentPtr == IntPtr.Zero) return table;

        vec3 currentVec = GetCurrentVec3();
        void AddAxis(string label, Func<vec3, float> getter, Action<float> setter)
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

            var memberLimits = _member.GetCustomAttribute<NumericUpDownLimitsAttribute>();

            float currentValue = getter(currentVec);
            var num = new NumericUpDown
            {
                DecimalPlaces = memberLimits?.DecimalPlaces ?? 4,
                Increment = memberLimits != null ? Convert.ToDecimal(memberLimits.Increment) : 0.1m,
                Minimum = memberLimits != null ? Convert.ToDecimal(memberLimits.Minimum) : -10000000m,
                Maximum = memberLimits != null ? Convert.ToDecimal(memberLimits.Maximum) : 10000000m,
                Value = (decimal)Math.Clamp(currentValue,
                    memberLimits?.Minimum ?? -10000000f,
                    memberLimits?.Maximum ?? 10000000f),
                Dock = DockStyle.Fill,
                BackColor = Color.FromArgb(60, 60, 60),
                ForeColor = Color.White,
                ReadOnly = _readOnly,
                Enabled = !_readOnly
            };

            num.ValueChanged += (s, e) =>
            {
                if (_readOnly) return;
                setter((float)num.Value);
            };

            num.TextChanged += (s, e) =>
            {
                if (_readOnly) return;
                setter((float)num.Value);
            };

            table.Controls.Add(num, 1, row);
        }

        // Use full struct replacement (safest with GlmSharp vec3 union)
        AddAxis("X", v => v.x, val =>
        {
            ref vec3 r = ref GetVec3Ref();
            r = new vec3(val, r.y, r.z);
        });

        AddAxis("Y", v => v.y, val =>
        {
            ref vec3 r = ref GetVec3Ref();
            r = new vec3(r.x, val, r.z);
        });

        AddAxis("Z", v => v.z, val =>   // ← You were missing Z!
        {
            ref vec3 r = ref GetVec3Ref();
            r = new vec3(r.x, r.y, val);
        });

        return table;
    }

    // Fixed and simplified
    private ref vec3 GetVec3Ref()
    {
        if (_wrapper == null
            || _wrapper.ComponentPtr == IntPtr.Zero
            || _member is not FieldInfo fieldInfo)
        {
            return ref Unsafe.NullRef<vec3>();
        }

        // This is the most reliable way for unmanaged structs (including those with unions)
        int offset = Marshal.OffsetOf(_wrapper.ComponentStructType, fieldInfo.Name).ToInt32();

        byte* basePtr = (byte*)_wrapper.ComponentPtr.ToPointer();   // This line caused your CS1503

        return ref Unsafe.AsRef<vec3>(basePtr + offset);
    }

    private vec3 GetCurrentVec3()
    {
        if (_wrapper == null) return vec3.Zero;

        var value = _wrapper.GetMemberValue(_member);
        return value is vec3 vec ? vec : vec3.Zero;
    }
}