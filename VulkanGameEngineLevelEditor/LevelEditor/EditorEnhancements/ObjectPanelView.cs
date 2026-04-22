using GlmSharp;
using System;
using System.ComponentModel;
using System.Drawing;
using System.Linq;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Windows.Forms;
using VulkanGameEngineLevelEditor.GameEngine;
using VulkanGameEngineLevelEditor.LevelEditor.Attributes;
using VulkanGameEngineLevelEditor.LevelEditor.ControlSubForms;
using VulkanGameEngineLevelEditor.LevelEditor.Registries;

namespace VulkanGameEngineLevelEditor.LevelEditor.EditorEnhancements
{
    public class ObjectPanelView : TableLayoutPanel
    {
        private const int HeaderHeight = 34;
        private const int RowHeight = 32;

        private readonly PropertiesPanel _propertiesPanel;
        private readonly ToolTip _toolTip;

        public object PanelObject { get; private set; }

        protected Panel _headerPanel;
        private Panel _contentPanel;
        private TableLayoutPanel _propTable;

        private bool _isExpanded = true;

        public ObjectPanelView(PropertiesPanel propertiesPanel, object component, ToolTip toolTip)
        {
            _propertiesPanel = propertiesPanel ?? throw new ArgumentNullException(nameof(propertiesPanel));
            PanelObject = component ?? throw new ArgumentNullException(nameof(component));
            _toolTip = toolTip ?? new ToolTip();

            InitializeLayout();
            CreateHeader();
            CreateContent();
            PopulateProperties();
        }

        public void RefreshValues()
        {
            if (PanelObject == null) return;
            for (int row = 0; row < _propTable.RowCount; row++)
            {
                Control? valueControl = _propTable.GetControlFromPosition(1, row);
                if (valueControl == null) continue;
                if (valueControl.Tag is not MemberInfo member) continue;
                object? newValue = GetCurrentValue(member);
                if (newValue == null) continue;
                UpdateControlValue(valueControl, newValue);
            }
        }

        private void InitializeLayout()
        {
            this.SuspendLayout();
            this.AutoSize = true;
            this.AutoSizeMode = AutoSizeMode.GrowAndShrink;
            this.ColumnCount = 1;
            this.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 100F));
            this.BackColor = Color.FromArgb(32, 32, 35);
            this.Margin = new Padding(6, 5, 6, 8);
            this.ResumeLayout(false);
        }

        private void CreateHeader()
        {
            _headerPanel = new Panel
            {
                Dock = DockStyle.Top,
                Height = HeaderHeight,
                BackColor = Color.FromArgb(52, 52, 58),
                Cursor = Cursors.Hand
            };

            var foldBtn = new Button
            {
                Text = _isExpanded ? "▼" : "▶",
                Dock = DockStyle.Left,
                Width = 28,
                FlatStyle = FlatStyle.Flat,
                BackColor = Color.Transparent,
                ForeColor = Color.White,
                Font = new Font("Segoe UI", 9.5f, FontStyle.Bold)
            };
            foldBtn.FlatAppearance.BorderSize = 0;
            foldBtn.Click += (s, e) => ToggleExpansion(foldBtn);

            var nameLabel = new Label
            {
                Text = GetDisplayName(),
                Dock = DockStyle.Fill,
                TextAlign = ContentAlignment.MiddleLeft,
                ForeColor = Color.White,
                Font = new Font("Segoe UI", 9.75f, FontStyle.Bold),
                Padding = new Padding(34, 0, 0, 0)
            };

            var removeBtn = new Button
            {
                Text = "×",
                Size = new Size(28, 28),
                Anchor = AnchorStyles.Top | AnchorStyles.Right,
                FlatStyle = FlatStyle.Flat,
                BackColor = Color.FromArgb(190, 55, 55),
                ForeColor = Color.White
            };
            removeBtn.FlatAppearance.BorderSize = 0;
            removeBtn.Click += (s, e) => _propertiesPanel.RemoveComponent(PanelObject);

            _headerPanel.Controls.Add(foldBtn);
            _headerPanel.Controls.Add(nameLabel);
            _headerPanel.Controls.Add(removeBtn);

            this.Controls.Add(_headerPanel);
        }

        private void CreateContent()
        {
            _contentPanel = new Panel
            {
                Dock = DockStyle.Fill,
                AutoSize = true,
                Visible = _isExpanded,
                BackColor = Color.FromArgb(28, 28, 31),
                Padding = new Padding(10, 8, 10, 10)
            };

            _propTable = new TableLayoutPanel
            {
                Dock = DockStyle.Fill,
                AutoSize = true,
                ColumnCount = 2,
                ColumnStyles =
                {
                    new ColumnStyle(SizeType.Percent, 37F),
                    new ColumnStyle(SizeType.Percent, 63F)
                }
            };

            _contentPanel.Controls.Add(_propTable);
            this.Controls.Add(_contentPanel);
        }

        private void ToggleExpansion(Button foldBtn)
        {
            _isExpanded = !_isExpanded;
            foldBtn.Text = _isExpanded ? "▼" : "▶";
            _contentPanel.Visible = _isExpanded;
            _propertiesPanel?.RefreshLayout();
        }

        private string GetDisplayName()
        {
            if (PanelObject is DynamicComponentWrapper wrapper)
            {
                var attr = wrapper.ComponentStructType.GetCustomAttribute<DisplayNameAttribute>();
                if (attr != null) return attr.DisplayName;
                return wrapper.ComponentType.ToString().Replace("k", "").Replace("Component", "");
            }

            var normalAttr = PanelObject.GetType().GetCustomAttribute<DisplayNameAttribute>();
            return normalAttr?.DisplayName ?? PanelObject.GetType().Name;
        }

        private void PopulateProperties()
        {
            _propTable.SuspendLayout();
            _propTable.Controls.Clear();
            _propTable.RowCount = 0;
            _propTable.RowStyles.Clear();

            if (PanelObject is DynamicComponentWrapper wrapper)
                AddPropertyControlsForWrapper(wrapper);
            else
                AddPropertyControlsForPlainObject(PanelObject);

            _propTable.ResumeLayout(true);
        }

        private void AddPropertyControlsForWrapper(DynamicComponentWrapper wrapper)
        {
            if (wrapper == null) return;

            var members = wrapper.ComponentStructType
                .GetMembers(BindingFlags.Public | BindingFlags.Instance)
                .Where(m => m is FieldInfo || (m is PropertyInfo p && p.CanRead))
                .OrderBy(m => m.Name)
                .ToList();

            foreach (var member in members)
            {
                if (ShouldIgnoreMember(member)) continue;

                var linkerAttr = member.GetCustomAttribute<ObjectLinkerAttribute>();
                object rawValue = GetCurrentMemberValue(wrapper, member);
                if (linkerAttr != null && rawValue != null)
                {
                    object handle = rawValue;
                    if (linkerAttr.HandleType == typeof(PointLightHandle) && rawValue is uint pid) handle = new PointLightHandle(new IntPtr(pid));
                    else if (linkerAttr.HandleType == typeof(DirectionalLightHandle) && rawValue is uint did) handle = new DirectionalLightHandle(new IntPtr(did));

                    object result = ObjectLinkerRegistry.Resolve(linkerAttr.HandleType, handle);
                    if (result is IntPtr ptr && ptr != IntPtr.Zero)
                    {
                        Type resolvedType = GetResolvedTypeFromHandle(linkerAttr.HandleType);
                        object linkedObject = Marshal.PtrToStructure(ptr, resolvedType);

                        if (linkedObject != null)
                        {
                            AddLinkedObjectHeader(resolvedType.Name);

                            var linkedPanel = new ObjectPanelView(_propertiesPanel, linkedObject, _toolTip);
                            if (linkedPanel._headerPanel != null) linkedPanel._headerPanel.Visible = false;

                            int linkedRow = _propTable.RowCount++;
                            _propTable.RowStyles.Add(new RowStyle(SizeType.AutoSize));

                            linkedPanel.Dock = DockStyle.Fill;
                            linkedPanel.Margin = new Padding(0, 6, 0, 12);
                            _propTable.Controls.Add(linkedPanel, 0, linkedRow);
                            _propTable.SetColumnSpan(linkedPanel, 2);

                            continue;
                        }
                    }
                }

                Type memberType = member is FieldInfo f ? f.FieldType : ((PropertyInfo)member).PropertyType;

                if (memberType == null || memberType == typeof(ulong) ||
                    memberType.IsPointer || memberType == typeof(IntPtr))
                {
                    continue;
                }

                if (IsDuplicateProperty(member.Name)) continue;

                int row = _propTable.RowCount++;
                _propTable.RowStyles.Add(new RowStyle(SizeType.AutoSize, RowHeight));

                var label = new Label
                {
                    Text = member.GetCustomAttribute<DisplayNameAttribute>()?.DisplayName ?? FormatPropertyName(member.Name),
                    Dock = DockStyle.Fill,
                    TextAlign = ContentAlignment.MiddleLeft,
                    ForeColor = Color.White,
                    Margin = new Padding(6, 4, 4, 4)
                };

                _propTable.Controls.Add(label, 0, row);

                bool isReadOnly = true;

                if (member is PropertyInfo pi)
                {
                    isReadOnly = pi.GetCustomAttribute<ReadOnlyAttribute>()?.IsReadOnly ?? !pi.CanWrite;
                }
                else if (member is FieldInfo)
                {
                    isReadOnly = member.GetCustomAttribute<ReadOnlyAttribute>()?.IsReadOnly ?? false;
                }

                var control = ControlRegistry.CreateControl(this, memberType, wrapper, member, RowHeight, isReadOnly);
                if (control != null)
                {
                    control.Tag = member;
                    control.Dock = DockStyle.Fill;
                    control.Margin = new Padding(5);
                    _propTable.Controls.Add(control, 1, row);
                }
            }
        }

        private void AddLinkedObjectHeader(string resolvedTypeName)
        {
            int headerRow = _propTable.RowCount++;
            _propTable.RowStyles.Add(new RowStyle(SizeType.Absolute, 34));

            var header = new Label
            {
                Text = $"→ {resolvedTypeName}",
                ForeColor = Color.Cyan,
                Font = new Font("Segoe UI", 10f, FontStyle.Bold),
                Dock = DockStyle.Fill,
                TextAlign = ContentAlignment.MiddleLeft,
                Margin = new Padding(12, 6, 4, 4),
                BackColor = Color.FromArgb(45, 45, 65)
            };

            _propTable.Controls.Add(header, 0, headerRow);
            _propTable.SetColumnSpan(header, 2);
        }

        private bool IsDuplicateProperty(string name)
        {
            return _propTable.Controls.OfType<Label>()
                .Any(l => string.Equals(l.Text, name, StringComparison.OrdinalIgnoreCase));
        }

        private string FormatPropertyName(string name)
        {
            return name;
        }

        private void AddPropertyControlsForPlainObject(object targetObject)
        {
            if (targetObject == null) return;

            var members = targetObject.GetType()
                .GetMembers(BindingFlags.Public | BindingFlags.Instance)
                .Where(m => m is FieldInfo || (m is PropertyInfo p && p.CanRead))
                .OrderBy(m => m.Name);

            foreach (var member in members)
            {
                if (ShouldIgnoreResolvedMember(member)) continue;

                Type memberType = member is FieldInfo f ? f.FieldType : (member is PropertyInfo p ? p.PropertyType : null);
                if (memberType == null) continue;

                string displayName = member.GetCustomAttribute<DisplayNameAttribute>()?.DisplayName ?? member.Name;
                bool isReadOnly = member.GetCustomAttribute<ReadOnlyAttribute>()?.IsReadOnly ?? false;

                var control = ControlRegistry.CreateControl(this, memberType, targetObject, member, RowHeight, isReadOnly);
                if (control != null)
                {
                    int row = _propTable.RowCount++;
                    _propTable.RowStyles.Add(new RowStyle(SizeType.AutoSize, RowHeight));

                    var label = new Label
                    {
                        Text = displayName,
                        Dock = DockStyle.Fill,
                        TextAlign = ContentAlignment.MiddleLeft,
                        ForeColor = Color.LightGray,
                        Margin = new Padding(6, 4, 4, 4)
                    };

                    _propTable.Controls.Add(label, 0, row);
                    control.Tag = member;
                    control.Dock = DockStyle.Fill;
                    control.Margin = new Padding(5);
                    _propTable.Controls.Add(control, 1, row);
                }
            }
        }

        private static bool ShouldIgnoreMember(MemberInfo member)
        {
            return member.GetCustomAttribute<IgnorePropertyAttribute>() != null ||
                   member.Name == "GameObjectId" ||
                   member.Name == "ComponentType" ||
                   member.Name == "DisplayName";
        }

        private object GetCurrentMemberValue(DynamicComponentWrapper wrapper, MemberInfo member)
        {
            try { return wrapper.GetMemberValue(member); }
            catch { return null; }
        }

        private static bool ShouldIgnoreResolvedMember(MemberInfo member)
        {
            return member.Name == "GameObjectId" ||
                   member.GetCustomAttribute<IgnorePropertyAttribute>() != null;
        }

        private Type GetResolvedTypeFromHandle(Type handleType)
        {
            if (handleType == typeof(PointLightHandle)) return typeof(PointLight);
            if (handleType == typeof(DirectionalLightHandle)) return typeof(DirectionalLight);
            return typeof(object);
        }

        private object? GetCurrentValue(MemberInfo member)
        {
            try
            {
                if (PanelObject is DynamicComponentWrapper wrapper) return wrapper.GetMemberValue(member);
                if (member is FieldInfo field) return field.GetValue(PanelObject);
                if (member is PropertyInfo prop && prop.CanRead) return prop.GetValue(PanelObject);
                return null;
            }
            catch
            {
                return null;
            }
        }

        private void UpdateControlValue(Control control, object value)
        {
            if (control is NumericUpDown num)
            {
                if (value is float f) num.Value = (decimal)Math.Clamp(f, (double)num.Minimum, (double)num.Maximum);
                else if (value is double d) num.Value = (decimal)Math.Clamp(d, (double)num.Minimum, (double)num.Maximum);
                else if (value is int i) num.Value = i;
                else if (value is uint u) num.Value = u;
                return;
            }

            if (control is TableLayoutPanel table)
            {
                var nums = table.Controls.OfType<NumericUpDown>().ToList();
                if (value is vec2 vec2Value)
                {
                    if (nums.Count >= 2)
                    {
                        nums[0].Value = (decimal)vec2Value.x; 
                        nums[1].Value = (decimal)vec2Value.y;
                    }
                }
                else if (value is vec3 vec3Value)
                {
                    if (nums.Count >= 3)
                    {
                        nums[0].Value = (decimal)vec3Value.x; 
                        nums[1].Value = (decimal)vec3Value.y;
                        nums[2].Value = (decimal)vec3Value.z;
                    }
                }
                return;
            }

            if (control is CheckBox chk)
            {
                if (value is bool b)
                    chk.Checked = b;
                return;
            }

            if (control is TextBox txt)
            {
                txt.Text = value?.ToString() ?? "";
                return;
            }
        }
    }
}