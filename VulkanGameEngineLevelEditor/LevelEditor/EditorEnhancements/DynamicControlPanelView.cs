//using System;
//using System.Collections.Generic;
//using System.Drawing;
//using System.Linq;
//using System.Reflection;
//using System.Windows.Forms;
//using VulkanGameEngineLevelEditor.GameEngine;
//using VulkanGameEngineLevelEditor.LevelEditor.Attributes;

//namespace VulkanGameEngineLevelEditor.LevelEditor.EditorEnhancements
//{
//    public class DynamicControlPanelView : TableLayoutPanel
//    {
//        private readonly LevelEditorForm _levelEditorForm;
//        private object _selectedObject;
//        public object SelectedObject
//        {
//            get => _selectedObject;
//        }
//        private readonly ToolTip _toolTip;

//        // We only need one map now: object → its panel
//        private readonly Dictionary<object, ObjectPanelView> _objectPanelMap = new();

//        public void SetSelectedObject(object value)
//        {
//            if (_selectedObject != value)
//            {
//                _selectedObject = value;
//                RefreshPanels();        // renamed from UpdatePanels for clarity
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
//            this.Padding = new Padding(4);

//            _contentPanel = new TableLayoutPanel
//            {
//                Dock = DockStyle.Fill,
//                AutoScroll = true,
//                AutoSize = true,
//                BackColor = Color.FromArgb(40, 40, 40),
//                ColumnCount = 1,
//                ColumnStyles = { new ColumnStyle(SizeType.Percent, 100F) },
//                Padding = new Padding(4)
//            };

//            this.Controls.Add(_contentPanel);
//        }

//        public void SetSelectedObject(object obj)
//        {
//            if (SelectedObject == obj)
//                return;

//            SelectedObject = obj;
//            RefreshPanels();
//        }

//        private void RefreshPanels()
//        {
//            try
//            {
//                _contentPanel.Controls.Clear();
//                _objectPanelMap.Clear();

//                if (SelectedObject == null)
//                    return;

//                if (SelectedObject is GameObject go && go.GameObjectId != uint.MaxValue)
//                {
//                    // 1. Add the GameObject itself (header)
//                    AddObjectPanel(go);

//                    // 2. Add all attached components
//                    AddAllComponentPanels(go);
//                }
//                else
//                {
//                    // Regular object (component data, etc.)
//                    AddObjectPanel(SelectedObject);
//                }

//                AdjustHeight();
//            }
//            catch (Exception ex)
//            {
//                Console.WriteLine($"[DynamicControlPanel] RefreshPanels CRASH: {ex}");
//            }
//        }

//        private void AddObjectPanel(object obj)
//        {
//            var panel = new ObjectPanelView(_levelEditorForm, obj, _toolTip);
//            _contentPanel.Controls.Add(panel);
//            _objectPanelMap[obj] = panel;
//        }

//        private void AddAllComponentPanels(GameObject go)
//        {
//            var componentMembers = go.GetType()
//                .GetMembers(BindingFlags.Public | BindingFlags.Instance)
//                .Where(m => m.GetCustomAttribute<GameObjectComponentAttribute>() != null);

//            foreach (var member in componentMembers)
//            {
//                var attr = member.GetCustomAttribute<GameObjectComponentAttribute>();
//                if (attr == null) continue;

//                uint componentId = GetComponentId(member, go);
//                if (componentId == uint.MaxValue || componentId == 0)
//                    continue;

//                if (!ComponentRegistry.Finders.TryGetValue(attr.ComponentType, out var finder))
//                    continue;

//                object component = null;
//                try
//                {
//                    component = finder(componentId);
//                }
//                catch (Exception ex)
//                {
//                    Console.WriteLine($"[Component Finder Failed] {attr.ComponentType}: {ex.Message}");
//                }

//                if (component == null || _objectPanelMap.ContainsKey(component))
//                    continue;

//                try
//                {
//                    var panel = new ObjectPanelView(_levelEditorForm, component, _toolTip);
//                    _contentPanel.Controls.Add(panel);
//                    _objectPanelMap[component] = panel;

//                    Console.WriteLine($"[Inspector] Added component: {attr.ComponentType} (ID: {componentId})");
//                }
//                catch (Exception ex)
//                {
//                    Console.WriteLine($"[Inspector] Failed to create panel for {attr.ComponentType}: {ex}");
//                }
//            }
//        }

//        private static uint GetComponentId(MemberInfo member, GameObject go)
//        {
//            try
//            {
//                return member switch
//                {
//                    PropertyInfo p => (uint)p.GetValue(go),
//                    FieldInfo f => (uint)f.GetValue(go),
//                    _ => uint.MaxValue
//                };
//            }
//            catch
//            {
//                return uint.MaxValue;
//            }
//        }

//        private void AdjustHeight()
//        {
//            int totalHeight = _contentPanel.Controls.Cast<Control>()
//                                  .Sum(c => c.Height + c.Margin.Vertical) + 40;

//            _contentPanel.Height = Math.Max(totalHeight, 300);
//            this.PerformLayout();
//        }
//    }
//}