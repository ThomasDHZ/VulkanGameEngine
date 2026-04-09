//using System;
//using System.Collections.Generic;
//using System.Drawing;
//using System.Linq;
//using System.Windows.Forms;
//using VulkanGameEngineLevelEditor.GameEngine;
//using VulkanGameEngineLevelEditor.LevelEditor.EditorEnhancements;
//using VulkanGameEngineLevelEditor.LevelEditor.Forms;

//namespace VulkanGameEngineLevelEditor.LevelEditor.EditorEnhancements
//{
//    public class DynamicControlPanelView : TableLayoutPanel
//    {
//        private readonly LevelEditorForm _levelEditorForm;
//        private readonly ToolTip _toolTip;

//        private Entity? _selectedEntity;
//        private readonly Dictionary<object, ObjectPanelView> _objectPanelMap = new();

//        private TableLayoutPanel _contentPanel = null!;

//        public Entity? SelectedEntity
//        {
//            get => _selectedEntity;
//            set
//            {
//                if (_selectedEntity?.Id != value?.Id)   // Safe comparison by ID
//                {
//                    _selectedEntity = value;
//                    RefreshPanels();
//                }
//            }
//        }

//        public DynamicControlPanelView(LevelEditorForm form)
//        {
//            _levelEditorForm = form;
//            _toolTip = new ToolTip();
//            InitializeComponents();
//        }

//        private void InitializeComponents()
//        {
//            this.Dock = DockStyle.Right;
//            this.AutoScroll = true;
//            this.BackColor = Color.FromArgb(40, 40, 40);
//            this.Padding = new Padding(6);

//            _contentPanel = new TableLayoutPanel
//            {
//                Dock = DockStyle.Fill,
//                AutoScroll = true,
//                AutoSize = true,
//                BackColor = Color.FromArgb(40, 40, 40),
//                ColumnCount = 1,
//                ColumnStyles = { new ColumnStyle(SizeType.Percent, 100F) }
//            };

//            this.Controls.Add(_contentPanel);
//        }

//        private void RefreshPanels()
//        {
//            try
//            {
//                _contentPanel.Controls.Clear();
//                _objectPanelMap.Clear();

//                if (_selectedEntity == null || !_selectedEntity.IsValid())
//                {
//                    _contentPanel.Controls.Add(CreateNoSelectionLabel());
//                    return;
//                }

//                // 1. Entity Header (like Unity's GameObject header)
//                _contentPanel.Controls.Add(CreateEntityHeader());

//                // 2. Add Component Button (top level, like Unity)
//                _contentPanel.Controls.Add(CreateAddComponentButton());

//                // 3. One ObjectPanelView per Component (Unity-style collapsible sections)
//                var components = _selectedEntity.GetAllComponents();   // Your ECS method

//                foreach (var component in components)
//                {
//                    if (component == null) continue;

//                    var componentPanel = new ObjectPanelView(_levelEditorForm, component, _toolTip);
//                    _contentPanel.Controls.Add(componentPanel);
//                    _objectPanelMap[component] = componentPanel;
//                }

//                AdjustHeight();
//            }
//            catch (Exception ex)
//            {
//                Console.WriteLine($"[DynamicControlPanel] RefreshPanels error: {ex}");
//            }
//        }

//        private Label CreateNoSelectionLabel()
//        {
//            return new Label
//            {
//                Text = "No entity selected",
//                ForeColor = Color.Silver,
//                AutoSize = true,
//                Padding = new Padding(20),
//                Font = new Font("Segoe UI", 9f)
//            };
//        }

//        private Control CreateEntityHeader()
//        {
//            var header = new Panel
//            {
//                Height = 92,
//                Dock = DockStyle.Top,
//                BackColor = Color.FromArgb(48, 48, 52),
//                Padding = new Padding(12)
//            };

//            var nameLabel = new Label
//            {
//                Text = $"Entity: {_selectedEntity!.Name ?? $"Entity_{_selectedEntity.Id}"}",
//                Font = new Font("Segoe UI", 10.5f, FontStyle.Bold),
//                ForeColor = Color.White,
//                AutoSize = true,
//                Location = new Point(4, 10)
//            };

//            var idLabel = new Label
//            {
//                Text = $"ID: {_selectedEntity.Id}",
//                ForeColor = Color.Silver,
//                AutoSize = true,
//                Location = new Point(4, 34)
//            };

//            var enabledCheck = new CheckBox
//            {
//                Text = "Enabled",
//                Checked = _selectedEntity.Enabled,
//                ForeColor = Color.White,
//                Location = new Point(4, 60)
//            };
//            enabledCheck.CheckedChanged += (s, e) =>
//            {
//                if (_selectedEntity != null)
//                    _selectedEntity.Enabled = enabledCheck.Checked;
//            };

//            header.Controls.Add(nameLabel);
//            header.Controls.Add(idLabel);
//            header.Controls.Add(enabledCheck);
//            return header;
//        }

//        private Button CreateAddComponentButton()
//        {
//            var btn = new Button
//            {
//                Text = "＋ Add Component",
//                AutoSize = true,
//                Margin = new Padding(8, 8, 8, 16),
//                Padding = new Padding(8, 6, 8, 6),
//                BackColor = Color.FromArgb(70, 130, 70),
//                ForeColor = Color.White,
//                FlatStyle = FlatStyle.Flat,
//                Height = 38,
//                Font = new Font("Segoe UI", 9.5f)
//            };

//            btn.Click += (s, e) => ShowAddComponentDialog();
//            return btn;
//        }

//        private void ShowAddComponentDialog()
//        {
//            var componentTypes = ComponentRegistry.GetAllComponentTypes();

//            using var dialog = new Form
//            {
//                Text = "Add Component",
//                Size = new Size(360, 480),
//                StartPosition = FormStartPosition.CenterParent,
//                FormBorderStyle = FormBorderStyle.FixedDialog
//            };

//            var listBox = new ListBox
//            {
//                Dock = DockStyle.Fill,
//                Font = new Font("Segoe UI", 10f)
//            };

//            listBox.Items.AddRange(componentTypes.Select(t => t.Name).ToArray());

//            listBox.DoubleClick += (s, e) =>
//            {
//                if (listBox.SelectedItem is string selectedName && _selectedEntity != null)
//                {
//                    var type = componentTypes.FirstOrDefault(t => t.Name == selectedName);
//                    if (type != null)
//                    {
//                        var newComponent = Activator.CreateInstance(type);
//                        _selectedEntity.AddComponent(newComponent);
//                        dialog.Close();
//                        RefreshPanels();
//                    }
//                }
//            };

//            dialog.Controls.Add(listBox);
//            dialog.ShowDialog(this);
//        }

//        private void AdjustHeight()
//        {
//            int totalHeight = _contentPanel.Controls.Cast<Control>()
//                .Sum(c => c.Height + c.Margin.Vertical) + 120;

//            _contentPanel.Height = Math.Max(totalHeight, 500);
//            this.PerformLayout();
//        }

//        // Optional: Public refresh method (useful for live updates)
//        public void RefreshCurrentSelection()
//        {
//            RefreshPanels();
//        }
//    }
//}