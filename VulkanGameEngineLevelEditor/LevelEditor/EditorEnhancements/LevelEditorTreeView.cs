using System;
using System.Collections;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Linq;
using System.Reflection;
using System.Windows.Forms;
using Vulkan;
using VulkanGameEngineLevelEditor.GameEngine.Systems;
using VulkanGameEngineLevelEditor.LevelEditor.Attributes;
using VulkanGameEngineLevelEditor.LevelEditor.EditorEnhancements;
using MethodInvoker = System.Windows.Forms.MethodInvoker;

namespace VulkanGameEngineLevelEditor.LevelEditor
{
    public class LevelEditorTreeView : TreeView
    {
        public object _rootObject;
        public DynamicControlPanelView DynamicControlPanel;

        public LevelEditorTreeView()
        {
            InitializeComponents();
        }

        private void InitializeComponents()
        {
            this.AfterSelect += LevelEditorTreeView_AfterSelect;
        }

        public void PopulateTreeView(object rootObject)
        {
            _rootObject = rootObject;

            this.Nodes.Clear();
            if (rootObject == null)
            {
                return;
            }

            if (rootObject is IList list)
            {
                if (list.Count == 0)
                {
                    return;
                }

                foreach (var item in list)
                {
                    if (item != null)
                    {
                        TreeNode listNode = new TreeNode(item.GetType().Name)
                        {
                            Tag = item
                        };
                        this.Nodes.Add(listNode);
                        PopulateNode(listNode, item);
                    }
                }
            }
            else
            {
                TreeNode rootNode = new TreeNode(rootObject.GetType().Name)
                {
                    Tag = rootObject
                };
                PopulateNode(rootNode, rootObject);
                this.Nodes.Add(rootNode);
                rootNode.Expand();
            }
        }

        public static void PopulateNode(TreeNode parentNode, object parentObject)
        {
            if (parentObject == null) return;

            foreach (var prop in parentObject.GetType().GetProperties())
            {
                object value = prop.GetValue(parentObject);
                if (value == null) continue;

                if (typeof(IList).IsAssignableFrom(prop.PropertyType))
                {
                    if (IgnoreTypes(prop.PropertyType) ||
                        IgnoreProperties(prop))
                    {
                        continue;
                    }

                    var list = value as IList;
                    if (list != null &&
                        list.Count > 0)
                    {
                        var propertyDisplayNameAttr = prop.GetCustomAttribute<DisplayNameAttribute>();
                        string propertyDisplayName = propertyDisplayNameAttr?.DisplayName ?? prop.Name;

                        TreeNode listNode = new TreeNode(propertyDisplayName)
                        {
                            Tag = value
                        };
                        parentNode.Nodes.Add(listNode);

                        int index = 0;
                        foreach (var item in list)
                        {
                            if (item != null)
                            {
                                TreeNode itemNode = new TreeNode($"{item.GetType().Name}[{index}]")
                                {
                                    Tag = item
                                };
                                PopulateNode(itemNode, item);
                                listNode.Nodes.Add(itemNode);
                            }
                            index++;
                        }
                    }
                }
                else if(IgnoreTypes(prop.PropertyType) ||
                        IgnoreProperties(prop))
                {
                    continue;
                }
                else if (prop.PropertyType.IsClass && prop.PropertyType != typeof(string) ||
                         (prop.PropertyType.IsValueType && !prop.PropertyType.IsPrimitive))
                {
                    var propertyDisplayNameAttr = prop.GetCustomAttribute<DisplayNameAttribute>();
                    string propertyDisplayName = propertyDisplayNameAttr?.DisplayName ?? prop.Name;

                    TreeNode complexNode = new TreeNode(propertyDisplayName)
                    {
                        Tag = value
                    };
                    PopulateNode(complexNode, value);
                    parentNode.Nodes.Add(complexNode);
                }
            }

            if (parentObject is GameObject go)
            {
                var members = go.GetType().GetProperties(BindingFlags.Public | BindingFlags.Instance)
                                .Cast<MemberInfo>()
                                .Concat(go.GetType().GetFields(BindingFlags.Public | BindingFlags.Instance))
                                .Where(m => m.GetCustomAttribute<GameObjectComponentAttribute>() != null);

                foreach (var member in members)
                {
                    var attr = member.GetCustomAttribute<GameObjectComponentAttribute>();
                    uint id = 0;
                    try { id = (uint)(member is PropertyInfo p ? p.GetValue(go) : ((FieldInfo)member).GetValue(go)); }
                    catch { continue; }

                    if (id == uint.MaxValue) continue;

                    if (ComponentRegistry.Finders.TryGetValue(attr.ComponentType, out var finder))
                    {
                        try
                        {
                            var comp = finder(id);
                            if (comp != null)
                            {
                                TreeNode node = new TreeNode($"{attr.ComponentType} (ID: {id})") { Tag = comp };
                                PopulateNode(node, comp);
                                parentNode.Nodes.Add(node);
                            }
                        }
                        catch { }
                    }
                }
            }
        }

        private void LevelEditorTreeView_AfterSelect(object sender, TreeViewEventArgs e)
        {
            if (e.Node?.Tag == null)
            {
                Console.WriteLine("[AfterSelect] Node.Tag is null");
                return;
            }

            Console.WriteLine($"[AfterSelect] Selecting: {e.Node.Tag.GetType().Name} (Hash: {e.Node.Tag.GetHashCode()})");

            if (DynamicControlPanel != null)
            {
                this.BeginInvoke((MethodInvoker)delegate
                {
                    try
                    {
                        DynamicControlPanel.SelectedObject = e.Node.Tag;
                    }
                    catch (Exception ex)
                    {
                        Console.WriteLine($"[AfterSelect] CRASH in SelectedObject: {ex}");
                    }
                });
            }
        }

        private static bool IgnoreTypes(Type type)
        {
            return type == typeof(Guid) ||
                   type == typeof(IntPtr) ||
                   type.IsPointer; 
        }

        private static bool IgnoreProperties(PropertyInfo property)
        {
            Type propertyType = property.PropertyType;

            if (propertyType.IsEnum)
            {
                return true;
            }

            if (typeof(IEnumerable<string>).IsAssignableFrom(propertyType))
            {
                return true;
            }

            return false;
        }
    }
}