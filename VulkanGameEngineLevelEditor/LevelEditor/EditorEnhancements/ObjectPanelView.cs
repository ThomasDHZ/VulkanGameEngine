using AutoMapper.Execution;
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
        private Panel _contentPanel;
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
            this.Visible = true;

            var objectPanel = new Panel
            {
                Dock = DockStyle.Fill,
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
                Height = RowHeight,
                Visible = true
            };
            objectPanel.Controls.Add(_headerPanel);

            AddFoldoutButton(_headerPanel);

            var panelDisplayNameAttr = PanelObject.GetType().GetCustomAttribute<DisplayNameAttribute>();
            string panelDisplayName = panelDisplayNameAttr?.DisplayName ?? PanelObject.GetType().Name;
            var objLabel = new Label
            {
                Text = panelDisplayName,
                Dock = DockStyle.Fill,
                TextAlign = ContentAlignment.MiddleRight,
                ForeColor = Color.White,
                BackColor = Color.FromArgb(30, 30, 30),
                Margin = new Padding(25, 5, 5, 5)
            };
            _headerPanel.Controls.Add(objLabel);

            _contentPanel = new Panel
            {
                Dock = DockStyle.Fill,
                AutoSize = true,
                BackColor = Color.FromArgb(30, 30, 30),
                Visible = _isExpanded
            };
            objectPanel.Controls.Add(_contentPanel);

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
            _contentPanel.Controls.Add(_propTable);

            if (PanelObject is GameObject)
            {
                AddComponentButton(_contentPanel);
            }
        }

        private void AddFoldoutButton(Panel headerPanel)
        {
            var foldoutButton = new Button
            {
                Text = _isExpanded ? "▼" : "▶",
                Dock = DockStyle.Left,
                Width = 20,
                BackColor = Color.FromArgb(40, 40, 40),
                ForeColor = Color.White,
                FlatStyle = FlatStyle.Flat
            };
            foldoutButton.FlatAppearance.BorderSize = 0;
            foldoutButton.Click += (s, e) =>
            {
                _isExpanded = !_isExpanded;
                foldoutButton.Text = _isExpanded ? "▼" : "▶";
                _propTable.Visible = _isExpanded;
                AdjustPanelHeight();
            };
            headerPanel.Controls.Add(foldoutButton);
        }

        private void AddComponentButton(Panel contentPanel)
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
            contentPanel.Controls.Add(addComponentButton);
        }

        private void AddPropertyPanel(MemberInfo member, Panel controlPanel)
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

                    Control control = null;
                    Type type = member is PropertyInfo prop ? prop.PropertyType : ((FieldInfo)member).FieldType;
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
                    else if (type.IsClass && 
                             type != typeof(string))
                    {
                        AddPropertyPanel(member, controlPanel);
                        continue;
                    }
                    else if (type.IsValueType && 
                            !type.IsPrimitive && 
                            !type.IsEnum)
                    {
                        AddPropertyPanel(member, controlPanel);
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

                if (list != null)
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
                        var listPanel = new TableLayoutPanel
                        {
                            Dock = DockStyle.Fill,
                            AutoSize = true,
                            BackColor = Color.FromArgb(30, 30, 30),
                            ColumnCount = 1,
                            ColumnStyles = { new ColumnStyle(SizeType.Percent, 100F) }
                        };
                        controlPanel.Controls.Add(listPanel);

                        UpdateListPanel(listPanel, list, member, obj, elementType);
                    }
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine($"Error handling list for {member.Name}: {ex.Message}");
            }
        }

        private void UpdateListPanel(TableLayoutPanel listPanel, IList list, MemberInfo member, object obj, Type elementType)
        {
            try
            {
                listPanel.Controls.Clear();
                listPanel.RowCount = 0;
                listPanel.RowStyles.Clear();
                ChildObjectPanels.Clear();

                if (list != null && list.Count > 0)
                {
                    foreach (var element in list)
                    {
                        if (element == null) continue;

                        int rowIndex = listPanel.RowCount;
                        listPanel.RowCount += 1;
                        listPanel.RowStyles.Add(new RowStyle(SizeType.AutoSize));

                        var childPanelView = new ObjectPanelView(element, ToolTip);
                        ChildObjectPanels.Add(childPanelView);
                        listPanel.Controls.Add(childPanelView, 0, rowIndex);

                        childPanelView.PropertyChanged += (s, e) => NotifyPropertyChanged();
                    }
                }

                int buttonRowIndex = listPanel.RowCount;
                listPanel.RowCount += 1;
                listPanel.RowStyles.Add(new RowStyle(SizeType.AutoSize));

                var buttonPanel = new Panel
                {
                    Dock = DockStyle.Fill,
                    AutoSize = true,
                    BackColor = Color.FromArgb(30, 30, 30)
                };
                listPanel.Controls.Add(buttonPanel, 0, buttonRowIndex);

                var buttonLayout = new FlowLayoutPanel
                {
                    Dock = DockStyle.Fill,
                    AutoSize = true,
                    FlowDirection = FlowDirection.LeftToRight,
                    BackColor = Color.FromArgb(30, 30, 30)
                };
                buttonPanel.Controls.Add(buttonLayout);

                var addButton = new Button
                {
                    Text = "+",
                    Width = 30,
                    Height = RowHeight,
                    BackColor = Color.FromArgb(40, 40, 40),
                    ForeColor = Color.White,
                    FlatStyle = FlatStyle.Flat,
                };
                addButton.FlatAppearance.BorderSize = 0;
                addButton.Click += (s, e) =>
                {
                    try
                    {
                        Type listElementType = elementType.IsGenericType ? elementType.GetGenericArguments()[0] : typeof(object);
                        object newElement;
                        if (list is ListPtr<vec3> listPtr)
                        {
                            newElement = default(vec3);
                        }
                        else
                        {
                            newElement = Activator.CreateInstance(listElementType);
                        }
                        list.Add(newElement);
                        UpdateListPanel(listPanel, list, member, obj, elementType);
                        NotifyPropertyChanged();
                    }
                    catch (Exception ex)
                    {
                        Console.WriteLine($"Error adding list element: {ex.Message}");
                    }
                };
                buttonLayout.Controls.Add(addButton);

                var removeButton = new Button
                {
                    Text = "–",
                    Width = 30,
                    Height = RowHeight,
                    BackColor = Color.FromArgb(40, 40, 40),
                    ForeColor = Color.White,
                    FlatStyle = FlatStyle.Flat
                };
                removeButton.FlatAppearance.BorderSize = 0;
                removeButton.Click += (s, e) =>
                {
                    try
                    {
                        if (list.Count > 0)
                        {
                            if (list is ListPtr<vec3> listPtr)
                            {
                                listPtr.Remove((uint)(list.Count - 1));
                            }
                            else
                            {
                                list.RemoveAt(list.Count - 1);
                            }
                            UpdateListPanel(listPanel, list, member, obj, elementType);
                            NotifyPropertyChanged();
                        }
                    }
                    catch (Exception ex)
                    {
                        Console.WriteLine($"Error removing list element: {ex.Message}");
                    }
                };
                buttonLayout.Controls.Add(removeButton);
            }
            catch (Exception ex)
            {
                Console.WriteLine($"Error updating list panel: {ex.Message}");
            }
        }

        private void AdjustPanelHeight()
        {
            try
            {
                if (Parent is TableLayoutPanel parentTable)
                {
                    _headerPanel.Visible = true;
                    if (!_isExpanded)
                    {
                        int headerHeight = _headerPanel.Height;
                        int totalMarginPadding = this.Margin.Top + this.Margin.Bottom + this.Padding.Top + this.Padding.Bottom;
                        this.Height = headerHeight + totalMarginPadding;
                    }
                    else
                    {
                        this.AutoSize = true;
                    }

                    parentTable.AutoSize = true;
                    parentTable.PerformLayout();
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine($"Error adjusting panel height: {ex.Message}");
            }
        }

        public event EventHandler PropertyChanged;

        private void NotifyPropertyChanged()
        {
            PropertyChanged?.Invoke(this, EventArgs.Empty);
        }
    }
}