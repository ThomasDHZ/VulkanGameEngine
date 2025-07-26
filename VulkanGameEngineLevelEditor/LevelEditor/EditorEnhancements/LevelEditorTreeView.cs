using System;
using System.Collections;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Linq;
using System.Reflection;
using System.Windows.Forms;
using Vulkan;
using VulkanGameEngineLevelEditor.LevelEditor.EditorEnhancements;

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

        private void PopulateNode(TreeNode parentNode, object parentObject)
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
        }

        private void LevelEditorTreeView_AfterSelect(object sender, TreeViewEventArgs e)
        {
            if (DynamicControlPanel != null && e.Node.Tag != null)
            {
                DynamicControlPanel.SelectedObject = e.Node.Tag;
            }
        }

        private bool IsSimpleType(Type type)
        {
            return type == typeof(string)
                   || type == typeof(decimal)
                   || type == typeof(int)
                   || type == typeof(uint)
                   || type == typeof(float)
                   || type == typeof(double)
                   || type == typeof(bool)
                   || type == typeof(byte)
                   || type == typeof(sbyte)
                   || type == typeof(short)
                   || type == typeof(ushort)
                   || type == typeof(long)
                   || type == typeof(ulong)
                   || type == typeof(char);
        }

        private bool IgnoreTypes(Type type)
        {
            return type == typeof(Guid) ||
                   type == typeof(IntPtr) ||
                   type.IsPointer; 
        }

        private bool IgnoreProperties(PropertyInfo property)
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