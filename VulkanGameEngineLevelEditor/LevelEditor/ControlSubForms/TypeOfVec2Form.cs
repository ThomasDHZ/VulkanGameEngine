using GlmSharp;
using System;
using System.Drawing;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Windows.Forms;
using VulkanGameEngineLevelEditor.LevelEditor.ControlSubForms;
using VulkanGameEngineLevelEditor.LevelEditor.EditorEnhancements;

public unsafe class TypeOfVec2Form : PropertyEditorForm
{
    private const int RowHeight = 32;

    private readonly DynamicComponentWrapper? _wrapper;
    private readonly MemberInfo _member;

    public TypeOfVec2Form(ObjectPanelView rootPanel, object obj, MemberInfo member,
                          int minimumPanelSize, bool readOnly)
        : base(rootPanel, obj, member, minimumPanelSize, readOnly)
    {
        _member = member;
        _wrapper = obj as DynamicComponentWrapper;

        if (_wrapper == null)
            Console.WriteLine($"[TypeOfVec2Form] Warning: obj is not DynamicComponentWrapper for {member.Name}");
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

        if (_wrapper == null || _wrapper.ComponentPtr == IntPtr.Zero)
        {
            Console.WriteLine($"[TypeOfVec2Form] Failed to create control - wrapper or pointer is null");
            return table;
        }

        vec2 currentVec = GetCurrentVec2();

        void AddAxis(string label, Func<vec2, float> getter, Action<float> directSetter)
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

            var num = new NumericUpDown
            {
                Value = (decimal)Math.Clamp(getter(currentVec), -1000000f, 1000000f),
                DecimalPlaces = 4,
                Increment = 0.1m,
                Minimum = -1000000m,
                Maximum = 1000000m,
                Dock = DockStyle.Fill,
                BackColor = Color.FromArgb(60, 60, 60),
                ForeColor = Color.White,
                ReadOnly = _readOnly,
                Enabled = !_readOnly
            };

            num.ValueChanged += (s, e) =>
            {
                if (_readOnly) return;
                directSetter((float)num.Value);
            };

            table.Controls.Add(num, 1, row);
        }

        AddAxis("X", v => v.x, val => { ref vec2 r = ref GetVec2Ref(); r.x = val; });
        AddAxis("Y", v => v.y, val => { ref vec2 r = ref GetVec2Ref(); r.y = val; });

        Console.WriteLine($"[TypeOfVec2Form] Created successfully for {_member.Name} ({_wrapper.ComponentType})");
        return table;
    }

    private ref vec2 GetVec2Ref()
    {
        if (_wrapper == null || _wrapper.ComponentPtr == IntPtr.Zero || _member is not FieldInfo fieldInfo)
        {
            Console.WriteLine($"[TypeOfVec2Form] Cannot get ref - not a FieldInfo or null wrapper for {_member?.Name}");
            return ref Unsafe.NullRef<vec2>();
        }

        int offset = Marshal.OffsetOf(_wrapper.ComponentStructType, fieldInfo.Name).ToInt32();
        return ref Unsafe.AsRef<vec2>((byte*)_wrapper.ComponentPtr.ToPointer() + offset);
    }

    private vec2 GetCurrentVec2()
    {
        if (_wrapper == null) return vec2.Zero;

        var value = _wrapper.GetMemberValue(_member);
        return value is vec2 vec ? vec : vec2.Zero;
    }
}