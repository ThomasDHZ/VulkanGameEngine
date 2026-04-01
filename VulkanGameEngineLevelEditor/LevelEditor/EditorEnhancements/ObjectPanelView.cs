//using System;
//using System.Collections;
//using System.Collections.Generic;
//using System.ComponentModel;
//using System.Drawing;
//using System.Linq;
//using System.Reflection;
//using System.Windows.Forms;
//using VulkanGameEngineLevelEditor.GameEngine;
//using VulkanGameEngineLevelEditor.LevelEditor.Attributes;
//using VulkanGameEngineLevelEditor.LevelEditor.ControlSubForms;

//namespace VulkanGameEngineLevelEditor.LevelEditor.EditorEnhancements
//{
//    public class ObjectPanelView : TableLayoutPanel
//    {
//        private const int RowHeight = 32;

//        private readonly LevelEditorForm _levelEditorForm;
//        private readonly ToolTip _toolTip;

//        public object PanelObject { get; private set; }
//        public ObjectPanelView ParentPanel { get; private set; }

//        private TableLayoutPanel _propTable;
//        private Panel _headerPanel;
//        private Panel _contentPanel;

//        private bool _isExpanded = true;

//        public ObjectPanelView(LevelEditorForm form, object obj, ToolTip toolTip, ObjectPanelView parentPanel = null)
//        {
//            if (obj == null) throw new ArgumentNullException(nameof(obj));

//            _levelEditorForm = form;
//            PanelObject = obj;
//            ParentPanel = parentPanel;
//            _toolTip = toolTip ?? new ToolTip();

//            InitializeComponents();
//            PopulateProperties();
//        }

//        private void InitializeComponents()
//        {
//            this.AutoSize = true;
//            this.ColumnCount = 1;
//            this.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 100F));
//            this.BackColor = Color.FromArgb(30, 30, 30);
//            this.Padding = new Padding(4);

//            var outerPanel = new Panel
//            {
//                Dock = DockStyle.Fill,
//                AutoSize = true,
//                BackColor = Color.FromArgb(30, 30, 30),
//                BorderStyle = BorderStyle.FixedSingle,
//                Padding = new Padding(5)
//            };
//            this.Controls.Add(outerPanel);

//            // Header
//            _headerPanel = new Panel
//            {
//                Dock = DockStyle.Top,
//                Height = RowHeight,
//                BackColor = Color.FromArgb(45, 45, 45)
//            };
//            outerPanel.Controls.Add(_headerPanel);

//            AddFoldoutButton();
//            AddHeaderLabel();

//            // Content
//            _contentPanel = new Panel
//            {
//                Dock = DockStyle.Fill,
//                AutoSize = true,
//                BackColor = Color.FromArgb(30, 30, 30),
//                Visible = _isExpanded
//            };
//            outerPanel.Controls.Add(_contentPanel);

//            _propTable = new TableLayoutPanel
//            {
//                Dock = DockStyle.Fill,
//                AutoSize = true,
//                AutoScroll = true,
//                BackColor = Color.FromArgb(30, 30, 30),
//                ColumnCount = 2,
//                ColumnStyles =
//                {
//                    new ColumnStyle(SizeType.Percent, 35F),
//                    new ColumnStyle(SizeType.Percent, 65F)
//                },
//                Padding = new Padding(0, 8, 0, 0)
//            };
//            _contentPanel.Controls.Add(_propTable);

//            // Special button for GameObject
//            if (PanelObject is GameObject)
//            {
//                AddComponentButton();
//            }
//        }

//        private void AddHeaderLabel()
//        {
//            var displayNameAttr = PanelObject.GetType().GetCustomAttribute<DisplayNameAttribute>();
//            string displayName = displayNameAttr?.DisplayName ?? PanelObject.GetType().Name;

//            var label = new Label
//            {
//                Text = displayName,
//                Dock = DockStyle.Fill,
//                TextAlign = ContentAlignment.MiddleLeft,
//                ForeColor = Color.White,
//                Padding = new Padding(25, 0, 0, 0),
//                Font = new Font("Segoe UI", 9.5f, FontStyle.Bold)
//            };

//            _headerPanel.Controls.Add(label);
//        }

//        private void AddFoldoutButton()
//        {
//            var btn = new Button
//            {
//                Text = _isExpanded ? "▼" : "▶",
//                Dock = DockStyle.Left,
//                Width = 24,
//                FlatStyle = FlatStyle.Flat,
//                BackColor = Color.FromArgb(40, 40, 40),
//                ForeColor = Color.White
//            };
//            btn.FlatAppearance.BorderSize = 0;

//            btn.Click += (s, e) =>
//            {
//                _isExpanded = !_isExpanded;
//                btn.Text = _isExpanded ? "▼" : "▶";
//                _contentPanel.Visible = _isExpanded;
//                AdjustPanelHeight();
//            };

//            _headerPanel.Controls.Add(btn);
//        }

//        private void AddComponentButton()
//        {
//            var btn = new Button
//            {
//                Text = "＋ Add Component",
//                Dock = DockStyle.Bottom,
//                Height = RowHeight,
//                BackColor = Color.FromArgb(40, 40, 40),
//                ForeColor = Color.LimeGreen,
//                FlatStyle = FlatStyle.Flat
//            };
//            btn.FlatAppearance.BorderSize = 0;

//            btn.Click += (s, e) =>
//            {
//                // TODO: Open component selection dialog
//                MessageBox.Show("Component selection dialog coming soon...", "Add Component",
//                    MessageBoxButtons.OK, MessageBoxIcon.Information);
//            };

//            _contentPanel.Controls.Add(btn);
//        }

//        private void PopulateProperties()
//        {
//            _propTable.Controls.Clear();
//            _propTable.RowCount = 0;
//            _propTable.RowStyles.Clear();

//            AddPropertyControls(PanelObject, _propTable);
//        }

//        private void AddPropertyControls(object target, TableLayoutPanel table)
//        {
//            var members = target.GetType()
//                .GetMembers(BindingFlags.Public | BindingFlags.Instance)
//                .Where(m => m is PropertyInfo or FieldInfo);

//            foreach (var member in members)
//            {
//                if (ShouldIgnoreMember(member)) continue;

//                Type memberType = member is PropertyInfo p ? p.PropertyType : ((FieldInfo)member).FieldType;

//                if (memberType == typeof(ulong) || memberType.IsPointer || memberType == typeof(IntPtr))
//                    continue;

//                string displayName = member.GetCustomAttribute<DisplayNameAttribute>()?.DisplayName ?? member.Name;
//                bool isReadOnly = member.GetCustomAttribute<ReadOnlyAttribute>()?.IsReadOnly ?? false;

//                int row = table.RowCount++;
//                table.RowStyles.Add(new RowStyle(SizeType.AutoSize, RowHeight));

//                // Label
//                var label = new Label
//                {
//                    Text = displayName,
//                    Dock = DockStyle.Fill,
//                    TextAlign = ContentAlignment.MiddleLeft,
//                    ForeColor = Color.White,
//                    Margin = new Padding(6, 4, 4, 4)
//                };
//                table.Controls.Add(label, 0, row);

//                // Value control
//                var control = ControlRegistry.CreateControl(this, memberType, target, member, RowHeight, isReadOnly);
//                if (control != null)
//                {
//                    control.Dock = DockStyle.Fill;
//                    control.Margin = new Padding(5);
//                    table.Controls.Add(control, 1, row);
//                }
//                else
//                {
//                    Console.WriteLine($"[ObjectPanelView] No editor control for {member.Name} ({memberType.Name})");
//                }
//            }
//        }

//        private static bool ShouldIgnoreMember(MemberInfo member)
//        {
//            return member.GetCustomAttribute<IgnorePropertyAttribute>() != null ||
//                   member.GetCustomAttribute<GameObjectComponentAttribute>() != null;
//        }

//        private void AdjustPanelHeight()
//        {
//            if (Parent is TableLayoutPanel parentTable)
//            {
//                parentTable.PerformLayout();
//            }
//        }

//        public void NotifyPropertyChanged()
//        {
//            _levelEditorForm.BeginInvoke((System.Windows.Forms.MethodInvoker)(() =>
//            {
//                try
//                {
//                    // For now, just refresh the current panel
//                    PopulateProperties();
//                    AdjustPanelHeight();

//                    // You can later add proper two-way sync if needed
//                    _levelEditorForm.QuickUpdateRenderPass();
//                }
//                catch (Exception ex)
//                {
//                    Console.WriteLine($"[NotifyPropertyChanged] Error: {ex.Message}");
//                }
//            }));
//        }
//    }
//}