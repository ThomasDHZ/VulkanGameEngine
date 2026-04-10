using System;
using System.ComponentModel;
using System.Drawing;
using System.Linq;
using System.Reflection;
using System.Windows.Forms;
using VulkanGameEngineLevelEditor.GameEngine;
using VulkanGameEngineLevelEditor.LevelEditor.Attributes;
using VulkanGameEngineLevelEditor.LevelEditor.ControlSubForms;

namespace VulkanGameEngineLevelEditor.LevelEditor.EditorEnhancements
{
    public class ObjectPanelView : TableLayoutPanel
    {
        private const int HeaderHeight = 34;
        private const int RowHeight = 32;

        private readonly PropertiesPanel _propertiesPanel;
        private readonly ToolTip _toolTip;

        public object PanelObject { get; private set; }

        private Panel _headerPanel;
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

        private void InitializeLayout()
        {
            this.AutoSize = true;
            this.AutoSizeMode = AutoSizeMode.GrowAndShrink;
            this.ColumnCount = 1;
            this.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 100F));
            this.BackColor = Color.FromArgb(32, 32, 35);
            this.Margin = new Padding(6, 5, 6, 8);
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
                Location = new Point(_headerPanel.Width - 34, 3),
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
            _propTable.Controls.Clear();
            _propTable.RowCount = 0;
            _propTable.RowStyles.Clear();

            Console.WriteLine($"[ObjectPanelView] Populating for type: {PanelObject.GetType().Name}");

            if (PanelObject is DynamicComponentWrapper wrapper)
            {
                Console.WriteLine($"[ObjectPanelView] → Using wrapper for component: {wrapper.ComponentType}");
                AddPropertyControlsForWrapper(wrapper);
            }
            else
            {
              //  AddPropertyControls(PanelObject, _propTable);
            }
        }

        private void AddPropertyControlsForWrapper(DynamicComponentWrapper wrapper)
        {
            var members = wrapper.ComponentStructType
                .GetMembers(BindingFlags.Public | BindingFlags.Instance)
                .Where(m => m is FieldInfo || (m is PropertyInfo p && p.CanRead))
                .OrderBy(m => m.Name);

            foreach (var member in members)
            {
                if (ShouldIgnoreMember(member)) continue;

                Type memberType = member is FieldInfo f ? f.FieldType : ((PropertyInfo)member).PropertyType;

                if (memberType == typeof(ulong) || memberType.IsPointer || memberType == typeof(IntPtr))
                    continue;

                string displayName = member.GetCustomAttribute<DisplayNameAttribute>()?.DisplayName ?? member.Name;
                bool isReadOnly = member.GetCustomAttribute<ReadOnlyAttribute>()?.IsReadOnly ?? false;

                int row = _propTable.RowCount++;
                _propTable.RowStyles.Add(new RowStyle(SizeType.AutoSize, RowHeight));

                var label = new Label
                {
                    Text = displayName,
                    Dock = DockStyle.Fill,
                    TextAlign = ContentAlignment.MiddleLeft,
                    ForeColor = Color.White,
                    Margin = new Padding(6, 4, 4, 4)
                };
                _propTable.Controls.Add(label, 0, row);

                var control = ControlRegistry.CreateControl(this, memberType, wrapper, member, RowHeight, isReadOnly);
                if (control != null)
                {
                    control.Dock = DockStyle.Fill;
                    control.Margin = new Padding(5);
                    _propTable.Controls.Add(control, 1, row);
                }
                else
                {
                    Console.WriteLine($"[ObjectPanelView] No editor for {member.Name} ({memberType.Name})");
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
    }
}