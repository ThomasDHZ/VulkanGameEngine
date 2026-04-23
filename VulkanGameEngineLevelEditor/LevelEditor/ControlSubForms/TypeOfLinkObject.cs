using System;
using System.Drawing;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Windows.Forms;
using VulkanGameEngineLevelEditor.LevelEditor.Attributes;
using VulkanGameEngineLevelEditor.LevelEditor.ControlSubForms;
using VulkanGameEngineLevelEditor.LevelEditor.EditorEnhancements;

public unsafe class TypeOfLinkObject : PropertyEditorForm
{
    private readonly DynamicComponentWrapper? _wrapper;
    private readonly MemberInfo _member;
    private readonly LinkObjectAttribute _linkAttr;

    public TypeOfLinkObject(ObjectPanelView rootPanel, object obj, MemberInfo member,
                                int minimumPanelSize, bool readOnly)
        : base(rootPanel, obj, member, minimumPanelSize, readOnly)
    {
        _member = member;
        _wrapper = obj as DynamicComponentWrapper;
        _linkAttr = member.GetCustomAttribute<LinkObjectAttribute>()!;
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
        table.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 35));
        table.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 65));

        uint currentId = GetCurrentId();

        var lbl = new Label
        {
            Text = "Link",
            Dock = DockStyle.Fill,
            TextAlign = ContentAlignment.MiddleLeft,
            ForeColor = Color.White,
            Margin = new Padding(6, 0, 0, 0)
        };

        var combo = new ComboBox
        {
            DropDownStyle = ComboBoxStyle.DropDownList,
            Dock = DockStyle.Fill,
            BackColor = Color.FromArgb(60, 60, 60),
            ForeColor = Color.White,
            Enabled = !_readOnly
        };

        PopulateAvailableLinks(combo, currentId);

        combo.SelectedIndexChanged += (s, e) =>
        {
            if (_readOnly || combo.SelectedItem is not LinkItem item) return;

            SetLinkIdDirect(item.Id);

            // Important: Refresh the entire parent panel so the nested linked object updates
            if (_rootPanel != null)
                _rootPanel.RefreshValues();        // you'll need to make RefreshValues public or add a RefreshLinked() method
        };

        table.Controls.Add(lbl, 0, 0);
        table.Controls.Add(combo, 1, 0);

        return table;
    }

    private uint GetCurrentId()
    {
        var val = _wrapper?.GetMemberValue(_member);
        return val is uint id ? id : uint.MaxValue;
    }

    private void SetLinkIdDirect(uint newId)
    {
        if (_wrapper == null || _wrapper.ComponentPtr == IntPtr.Zero || _member is not FieldInfo fi)
            return;

        try
        {
            int offset = Marshal.OffsetOf(_wrapper.ComponentStructType, fi.Name).ToInt32();
            ref uint target = ref Unsafe.AsRef<uint>((byte*)_wrapper.ComponentPtr + offset);
            target = newId;
        }
        catch (Exception ex)
        {
            Console.WriteLine($"[Link Set] Failed for {_member.Name}: {ex.Message}");
        }
    }

    private void PopulateAvailableLinks(ComboBox combo, uint currentId)
    {
        combo.Items.Clear();
        combo.Items.Add(new LinkItem { Id = uint.MaxValue, Display = "(None)" });

        // TODO: Fill with real data from your systems
        // Example for lights:
        // if (_linkAttr.HandleType == typeof(PointLightHandle))
        //     foreach (var light in LightSystem.GetAllPointLights())
        //         combo.Items.Add(new LinkItem { Id = light.Id, Display = light.Name ?? $"PointLight_{light.Id}" });

        // Select current
        int index = 0;
        for (int i = 0; i < combo.Items.Count; i++)
        {
            if (combo.Items[i] is LinkItem li && li.Id == currentId)
            {
                index = i;
                break;
            }
        }
        combo.SelectedIndex = index;
    }

    private class LinkItem
    {
        public uint Id { get; set; }
        public string Display { get; set; } = "";
        public override string ToString() => Display;
    }
}