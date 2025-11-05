// LevelEditor/EditorEnhancements/DynamicControlPanelView.cs
using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Reflection;
using System.Windows.Forms;
using VulkanGameEngineLevelEditor.GameEngine.Systems;
using VulkanGameEngineLevelEditor.LevelEditor.Attributes;
using VulkanGameEngineLevelEditor.LevelEditor.EditorEnhancements;

namespace VulkanGameEngineLevelEditor.LevelEditor.EditorEnhancements
{
    public class DynamicControlPanelView : TableLayoutPanel
    {
        private LevelEditorForm levelEditorForm;
        private TableLayoutPanel _contentPanel;
        private ToolTip _toolTip;
        private List<ObjectPanelView> _objectPanelViewList = new();
        public static Dictionary<object, ObjectPanelView> ObjectPanelViewMap = new();
        public static Dictionary<MemberInfo, List<Attribute>> _dynamicAttributes = new Dictionary<MemberInfo, List<Attribute>>();
        public static object rootObject;

        public DynamicControlPanelView(LevelEditorForm form)
        {
            levelEditorForm = form;
            _toolTip = new ToolTip();
            InitializeComponents();
        }

        private void InitializeComponents()
        {
            this.Dock = DockStyle.Right;
            this.AutoScroll = true;
            this.BackColor = Color.FromArgb(40, 40, 40);

            _contentPanel = new TableLayoutPanel
            {
                Dock = DockStyle.Fill,
                AutoScroll = true,
                AutoSize = true,
                BackColor = Color.FromArgb(40, 40, 40),
                ColumnCount = 1,
                ColumnStyles = { new ColumnStyle(SizeType.Percent, 100F) }
            };
            this.Controls.Add(_contentPanel);
        }

        public object SelectedObject
        {
            get => rootObject;
            set
            {
                if (rootObject != value)
                {
                    rootObject = value;
                    UpdatePanels();
                }
            }
        }

        public void UpdatePanels()
        {
            try
            {
                _contentPanel.Controls.Clear();
                _objectPanelViewList.Clear();
                ObjectPanelViewMap.Clear();

                if (rootObject is GameObject go && go.GameObjectId != uint.MaxValue)
                {
                    Console.WriteLine($"[UpdatePanels] GameObject ID: {go.GameObjectId}");
                    AddPanel(new ObjectPanelView(levelEditorForm, go, _toolTip));
                    AddComponentPanels(go);
                }
                else if (rootObject != null)
                {
                    AddPanel(new ObjectPanelView(levelEditorForm, rootObject, _toolTip));
                }

                AdjustHeight();
            }
            catch (Exception ex)
            {
                Console.WriteLine($"[UpdatePanels] CRASH: {ex}");
            }
        }

        private void AddPanel(ObjectPanelView panel)
        {
            _objectPanelViewList.Add(panel);
            _contentPanel.Controls.Add(panel);
            ObjectPanelViewMap[panel.PanelObject] = panel;
        }

        private void AddComponentPanels(GameObject go)
        {
            var members = go.GetType()
                .GetMembers(BindingFlags.Public | BindingFlags.Instance)
                .Where(m => m.GetCustomAttribute<GameObjectComponentAttribute>() != null);

            foreach (var member in members)
            {
                var attr = member.GetCustomAttribute<GameObjectComponentAttribute>();
                uint id = uint.MaxValue;

                try
                {
                    id = member.MemberType switch
                    {
                        MemberTypes.Property => (uint)((PropertyInfo)member).GetValue(go),
                        MemberTypes.Field => (uint)((FieldInfo)member).GetValue(go),
                        _ => uint.MaxValue
                    };
                }
                catch (Exception ex)
                {
                    Console.WriteLine($"[AddComponentPanels] Failed to read {member.Name}: {ex.Message}");
                    continue;
                }

                if (id == uint.MaxValue) continue;

                if (!ComponentRegistry.Finders.TryGetValue(attr.ComponentType, out var finder))
                {
                    Console.WriteLine($"[AddComponentPanels] No finder for {attr.ComponentType}");
                    continue;
                }

                object comp = null;
                try { comp = finder(id); }
                catch (Exception ex) { Console.WriteLine($"[AddComponentPanels] Finder failed: {ex}"); }

                if (comp == null)
                {
                    Console.WriteLine($"[AddComponentPanels] Component NULL: {attr.ComponentType} (ID: {id})");
                    continue;
                }

                if (ObjectPanelViewMap.ContainsKey(comp))
                {
                    Console.WriteLine($"[AddComponentPanels] Already exists: {attr.ComponentType} (Hash: {comp.GetHashCode()})");
                    continue;
                }

                try
                {
                    var panel = new ObjectPanelView(levelEditorForm, comp, _toolTip);
                    AddPanel(panel);
                    Console.WriteLine($"[AddComponentPanels] ADDED {attr.ComponentType} (ID: {id}) Hash: {comp.GetHashCode()}");
                }
                catch (Exception ex)
                {
                    Console.WriteLine($"[AddComponentPanels] FAILED to create panel: {ex}");
                }
            }
        }

        private static bool IsDefaultStruct(object obj)
        {
            foreach (var f in obj.GetType().GetFields(BindingFlags.Public | BindingFlags.Instance))
            {
                var v = f.GetValue(obj);
                if (v is float fVal && fVal != 0) return false;
                if (v is int i && i != 0) return false;
                if (v is uint u && u != 0) return false;
            }
            return true;
        }

        private void AdjustHeight()
        {
            int h = _contentPanel.Controls.Cast<Control>().Sum(c => c.Height + c.Margin.Vertical);
            _contentPanel.Height = Math.Max(h + 20, 320);
            this.PerformLayout();
        }
    }
}