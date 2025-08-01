using GlmSharp;
using System;
using System.Collections;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Linq;
using System.Reflection;
using System.Windows.Forms;
using VulkanGameEngineLevelEditor.GameEngineAPI;
using VulkanGameEngineLevelEditor.LevelEditor.Attributes;
using VulkanGameEngineLevelEditor.LevelEditor.ControlSubForms;
using VulkanGameEngineLevelEditor.LevelEditor.Dialog;

namespace VulkanGameEngineLevelEditor.LevelEditor.EditorEnhancements
{
    public class ObjectPanelView : TableLayoutPanel
    {
        private const int RowHeight = 32;
        private const int MinimumPanelSize = 320;

        public object PanelObject { get; private set; }
        public ToolTip ToolTip { get; private set; }
        public List<ObjectPanelView> ChildObjectPanels { get; } = new List<ObjectPanelView>();
        private TableLayoutPanel _propTable;
        private Panel _headerPanel;
        private bool _isExpanded = true;

        public ObjectPanelView(object obj, ToolTip toolTip)
        {
            if (obj == null) throw new ArgumentNullException(nameof(obj));
            PanelObject = obj;
            ToolTip = toolTip ?? new ToolTip
            {
                BackColor = Color.FromArgb(50, 50, 50),
                ForeColor = Color.FromArgb(200, 200, 200)
            };

            InitializeComponents();
            PopulateProperties();
        }

        private void InitializeComponents()
        {
            this.AutoSize = true;
            this.ColumnCount = 1;
            this.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 100F));
            this.RowCount = 1;
            this.RowStyles.Add(new RowStyle(SizeType.AutoSize));
            this.BackColor = Color.FromArgb(30, 30, 30);

            var objectPanel = new Panel
            {
                Dock = DockStyle.Top,
                AutoSize = true,
                BackColor = Color.FromArgb(30, 30, 30),
                BorderStyle = BorderStyle.FixedSingle,
                Padding = new Padding(5)
            };
            this.Controls.Add(objectPanel, 0, 0);

            _headerPanel = new Panel
            {
                Dock = DockStyle.Top,
                BackColor = Color.FromArgb(30, 30, 30),
                Height = RowHeight
            };
            objectPanel.Controls.Add(_headerPanel);

            var panelDisplayNameAttr = PanelObject.GetType().GetCustomAttribute<DisplayNameAttribute>();
            string panelDisplayName = panelDisplayNameAttr?.DisplayName ?? PanelObject.GetType().Name;
            var objLabel = new Label
            {
                Text = panelDisplayName,
                Dock = DockStyle.Fill,
                TextAlign = ContentAlignment.MiddleLeft,
                ForeColor = Color.White,
                BackColor = Color.FromArgb(30, 30, 30),
                Margin = new Padding(5)
            };
            _headerPanel.Controls.Add(objLabel);

            _propTable = new TableLayoutPanel
            {
                Dock = DockStyle.Fill,
                AutoSize = true,
                AutoScroll = true,
                BackColor = Color.FromArgb(30, 30, 30),
                ColumnCount = 2,
                ColumnStyles = { new ColumnStyle(SizeType.Percent, 30F), new ColumnStyle(SizeType.Percent, 70F) },
                Padding = new Padding(0, RowHeight + 10, 0, 0)
            };
            objectPanel.Controls.Add(_propTable);

            if (PanelObject is GameObject)
            {
                AddComponentButton(objectPanel);
            }
        }

        private void AddComponentButton(Panel objectPanel)
        {
            var addComponentButton = new Button
            {
                Text = "Add Component",
                Dock = DockStyle.Bottom,
                Height = RowHeight,
                BackColor = Color.FromArgb(40, 40, 40),
                ForeColor = Color.White,
                FlatStyle = FlatStyle.Flat
            };
            addComponentButton.FlatAppearance.BorderSize = 0;
            addComponentButton.Click += (s, e) =>
            {
                MessageBox.Show("Add Component functionality not implemented yet.");
            };
            objectPanel.Controls.Add(addComponentButton);
        }

        private void PopulateProperties()
        {
            try
            {
                var properties = PanelObject.GetType().GetProperties(BindingFlags.Public | BindingFlags.Instance);
                var fields = PanelObject.GetType().GetFields(BindingFlags.Public | BindingFlags.Instance);
                var members = properties.Cast<MemberInfo>().Concat(fields).ToList();

                foreach (var member in members)
                {
                    if (member.GetCustomAttribute<IgnorePropertyAttribute>() != null) continue;

                    var propertyDisplayNameAttr = member.GetCustomAttribute<DisplayNameAttribute>();
                    string propertyDisplayName = propertyDisplayNameAttr?.DisplayName ?? member.Name;

                    var readOnlyAttr = member.GetCustomAttribute<ReadOnlyAttribute>();
                    bool isReadOnly = readOnlyAttr?.IsReadOnly ?? false;

                    var controlTypeAttr = member.GetCustomAttribute<ControlTypeAttribute>();
                    var toolTipAttr = member.GetCustomAttribute<TooltipAttribute>();

                    int propRowIndex = _propTable.RowCount;
                    _propTable.RowCount += 1;
                    _propTable.RowStyles.Add(new RowStyle(SizeType.AutoSize, RowHeight));

                    var labelPanel = new Panel
                    {
                        Dock = DockStyle.Fill,
                        BackColor = Color.FromArgb(50, 50, 50),
                        Margin = new Padding(2),
                        Height = RowHeight
                    };
                    _propTable.Controls.Add(labelPanel, 0, propRowIndex);

                    var label = new Label
                    {
                        Text = propertyDisplayName,
                        Dock = DockStyle.Fill,
                        TextAlign = ContentAlignment.MiddleLeft,
                        ForeColor = Color.White,
                        BackColor = Color.FromArgb(50, 50, 50),
                        Margin = new Padding(5)
                    };
                    if (toolTipAttr != null)
                    {
                        ToolTip.SetToolTip(label, toolTipAttr.Tooltip);
                    }
                    labelPanel.Controls.Add(label);

                    var controlPanel = new Panel
                    {
                        Dock = DockStyle.Fill,
                        BackColor = Color.FromArgb(50, 50, 50),
                        Margin = new Padding(2),
                        AutoSize = true
                    };
                    _propTable.Controls.Add(controlPanel, 1, propRowIndex);

                    Type type = member is PropertyInfo prop ? prop.PropertyType : ((FieldInfo)member).FieldType;

                    Control control = null;

                    if (controlTypeAttr != null && controlTypeAttr.ControlType == typeof(TypeOfFileLoader))
                    {
                        control = new TypeOfFileLoader("Shader Files (*.spv, *.vert, *.frag)|*.spv;*.vert;*.frag|All Files (*.*)|*.*");
                    }
                    else if (typeof(IList).IsAssignableFrom(type))
                    {
                        HandleList(member, PanelObject, controlPanel, propertyDisplayName);
                        continue;
                    }
                    else if (type == typeof(List<ComponentTypeEnum>))
                    {
                        control = DynamicControlPanelView.TypeOfComponentList(PanelObject, member as PropertyInfo, isReadOnly);
                    }
                    else if (type.IsClass && type != typeof(string))
                    {
                        object instance = member is PropertyInfo p ? p.GetValue(PanelObject) : ((FieldInfo)member).GetValue(PanelObject);
                        if (instance != null)
                        {
                            var classPanel = new TableLayoutPanel
                            {
                                Dock = DockStyle.Fill,
                                AutoSize = true,
                                BackColor = Color.FromArgb(30, 30, 30),
                                ColumnCount = 1,
                                ColumnStyles = { new ColumnStyle(SizeType.Percent, 100F) }
                            };
                            controlPanel.Controls.Add(classPanel);

                            int rowIndex = classPanel.RowCount;
                            classPanel.RowCount += 1;
                            classPanel.RowStyles.Add(new RowStyle(SizeType.AutoSize));

                            var childPanelView = new ObjectPanelView(instance, ToolTip);
                            ChildObjectPanels.Add(childPanelView);
                            classPanel.Controls.Add(childPanelView, 0, rowIndex);
                        }
                        continue;
                    }
                    else if (type.IsValueType && !type.IsPrimitive && !type.IsEnum)
                    {
                        object instance = member is PropertyInfo p ? p.GetValue(PanelObject) : ((FieldInfo)member).GetValue(PanelObject);
                        if (instance != null)
                        {
                            var structPanel = new TableLayoutPanel
                            {
                                Dock = DockStyle.Fill,
                                AutoSize = true,
                                BackColor = Color.FromArgb(30, 30, 30),
                                ColumnCount = 1,
                                ColumnStyles = { new ColumnStyle(SizeType.Percent, 100F) }
                            };
                            controlPanel.Controls.Add(structPanel);

                            int rowIndex = structPanel.RowCount;
                            structPanel.RowCount += 1;
                            structPanel.RowStyles.Add(new RowStyle(SizeType.AutoSize));

                            var childPanelView = new ObjectPanelView(instance, ToolTip);
                            ChildObjectPanels.Add(childPanelView);
                            structPanel.Controls.Add(childPanelView, 0, rowIndex);
                        }
                        continue;
                    }
                    else
                    {
                        control = ControlRegistry.CreateControl(type, PanelObject, member, RowHeight, isReadOnly);
                    }

                    if (control != null)
                    {
                        control.Dock = DockStyle.Fill;
                        control.Margin = new Padding(5);
                        controlPanel.Controls.Add(control);
                    }
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine($"Error populating properties for {PanelObject.GetType().Name}: {ex.Message}");
            }
        }

        private void HandleList(MemberInfo member, object obj, Panel controlPanel, string propertyDisplayName)
        {
            try
            {
                IList list = null;
                Type elementType = null;

                if (member is PropertyInfo prop)
                {
                    list = prop.GetValue(obj) as IList;
                    elementType = prop.PropertyType;
                }
                else if (member is FieldInfo field)
                {
                    list = field.GetValue(obj) as IList;
                    elementType = field.FieldType;
                }

                if (list != null && list.Count > 0)
                {
                    if (typeof(IEnumerable<string>).IsAssignableFrom(elementType))
                    {
                        var control = new TypeOfStringForm(obj, member, RowHeight, false).CreateControl();
                        if (control != null)
                        {
                            control.Dock = DockStyle.Fill;
                            control.Margin = new Padding(5);
                            controlPanel.Controls.Add(control);
                        }
                    }
                    else
                    {
                        // Create a TableLayoutPanel to stack all elements vertically
                        var listPanel = new TableLayoutPanel
                        {
                            Dock = DockStyle.Fill,
                            AutoSize = true,
                            BackColor = Color.FromArgb(30, 30, 30),
                            ColumnCount = 1,
                            ColumnStyles = { new ColumnStyle(SizeType.Percent, 100F) }
                        };
                        controlPanel.Controls.Add(listPanel);

                        // Add each element as an ObjectPanelView
                        foreach (var element in list)
                        {
                            if (element == null) continue;

                            int rowIndex = listPanel.RowCount;
                            listPanel.RowCount += 1;
                            listPanel.RowStyles.Add(new RowStyle(SizeType.AutoSize));

                            var childPanelView = new ObjectPanelView(element, ToolTip);
                            ChildObjectPanels.Add(childPanelView);
                            listPanel.Controls.Add(childPanelView, 0, rowIndex);
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine($"Error handling list for {member.Name}: {ex.Message}");
            }
        }
    }
}