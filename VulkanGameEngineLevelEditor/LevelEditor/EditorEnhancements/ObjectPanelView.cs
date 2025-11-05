using AutoMapper.Execution;
using GlmSharp;
using Microsoft.CodeAnalysis.CSharp.Syntax;
using Silk.NET.Core;
using Silk.NET.Vulkan;
using System;
using System.Collections;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Linq;
using System.Reflection;
using System.Reflection.Metadata;
using System.Runtime.CompilerServices;
using System.Windows.Forms;
using Vulkan;
using VulkanGameEngineLevelEditor;
using VulkanGameEngineLevelEditor.GameEngine.Structs;
using VulkanGameEngineLevelEditor.GameEngine.Systems;
using VulkanGameEngineLevelEditor.GameEngineAPI;
using VulkanGameEngineLevelEditor.LevelEditor.Attributes;
using VulkanGameEngineLevelEditor.LevelEditor.ControlSubForms;
using VulkanGameEngineLevelEditor.LevelEditor.Dialog;
using VulkanGameEngineLevelEditor.LevelEditor.EditorEnhancements;
using VulkanGameEngineLevelEditor.Models;

namespace VulkanGameEngineLevelEditor.LevelEditor.EditorEnhancements
{
    public class ObjectPanelView : TableLayoutPanel
    {
        private const int RowHeight = 32;
        private const int MinimumPanelSize = 320;
        LevelEditorForm levelEditorForm { get; set; }
        public object PanelObject { get; private set; }
        public ToolTip ToolTip { get; private set; }
        private ObjectPanelView _parentPanel;
        public List<ObjectPanelView> ChildObjectPanels { get; } = new List<ObjectPanelView>();

        private TableLayoutPanel _propTable;
        private Panel _headerPanel;
        private Panel _contentPanel;
        private bool _isExpanded = true; 

        public ObjectPanelView(LevelEditorForm form, object obj, ToolTip toolTip, ObjectPanelView parentPanel = null)
        {
            if (obj == null) throw new ArgumentNullException(nameof(obj));
            DynamicControlPanelView.ObjectPanelViewMap[obj] = this;
            _parentPanel = parentPanel;
            PanelObject = obj;
            levelEditorForm = form;
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

   
        private void PopulateProperties()
        {
            try
            {
                _propTable.Controls.Clear();
                _propTable.RowCount = 0;
                _propTable.RowStyles.Clear();
                ChildObjectPanels.Clear();

                AddPropertyControls(PanelObject, _propTable, ChildObjectPanels);
            }
            catch (Exception ex)
            {
                Console.WriteLine($"Error populating properties for {PanelObject.GetType().Name}: {ex.Message}");
            }
        }

        private void AddPropertyControls(object targetObject, TableLayoutPanel targetTable, List<ObjectPanelView> childPanels)
        {
            try
            {
                var members = targetObject.GetType()
                    .GetMembers(BindingFlags.Public | BindingFlags.Instance)
                    .Where(m => m.MemberType == MemberTypes.Property || m.MemberType == MemberTypes.Field);

                foreach (var member in members)
                {
                    if (GetCustomAttribute<IgnorePropertyAttribute>(member) != null) continue;
                    if (GetCustomAttribute<GameObjectComponentAttribute>(member) != null) continue;

                    Type type = member.MemberType == MemberTypes.Property
                        ? ((PropertyInfo)member).PropertyType
                        : ((FieldInfo)member).FieldType;

                    if (type == typeof(UInt64) || type == typeof(void*) || type.IsPointer) continue;

                    var displayName = GetCustomAttribute<DisplayNameAttribute>(member)?.DisplayName ?? member.Name;
                    var readOnly = GetCustomAttribute<ReadOnlyAttribute>(member)?.IsReadOnly ?? false;

                    int row = targetTable.RowCount++;
                    targetTable.RowStyles.Add(new RowStyle(SizeType.AutoSize, RowHeight));

                    var label = new Label
                    {
                        Text = displayName,
                        Dock = DockStyle.Fill,
                        TextAlign = ContentAlignment.MiddleLeft,
                        ForeColor = Color.White,
                        Margin = new Padding(5)
                    };
                    targetTable.Controls.Add(label, 0, row);

                    Control control = ControlRegistry.CreateControl(this, type, targetObject, member, RowHeight, readOnly);
                    if (control != null)
                    {
                        control.Dock = DockStyle.Fill;
                        control.Margin = new Padding(5);
                        targetTable.Controls.Add(control, 1, row);
                    }
                    else
                    {
                        Console.WriteLine($"[WARN] No control for {member.Name} ({type.Name})");
                    }
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine($"[AddPropertyControls] ERROR: {ex}");
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

                if (obj is GameObject gameObject && list is IEnumerable<ComponentTypeEnum> listType)
                {
                    foreach (ComponentTypeEnum component in listType)
                    {
                        int rowIndex = listPanel.RowCount;
                        listPanel.RowCount += 1;
                        listPanel.RowStyles.Add(new RowStyle(SizeType.AutoSize));

                        var componentPanel = new TableLayoutPanel
                        {
                            Dock = DockStyle.Fill,
                            AutoSize = true,
                            BackColor = Color.FromArgb(30, 30, 30),
                            ColumnCount = 2,
                            ColumnStyles = { new ColumnStyle(SizeType.Percent, 30F), new ColumnStyle(SizeType.Percent, 70F) }
                        };
                        listPanel.Controls.Add(componentPanel, 0, rowIndex);

                        switch (component)
                        {
                            case ComponentTypeEnum.kTransform2DComponent:
                            //    object transformComponent = GameObjectSystem.Transform2DComponentMap[gameObject.GameObjectId];
                             //   AddPropertyControls(transformComponent, componentPanel, ChildObjectPanels);
                                break;
                            case ComponentTypeEnum.kInputComponent:
                           //     object inputComponent = GameObjectSystem.InputComponentMap[gameObject.GameObjectId];
                            //    AddPropertyControls(inputComponent, componentPanel, ChildObjectPanels);
                                break;
                            case ComponentTypeEnum.kSpriteComponent:
                             //   Sprite componentObject = SpriteSystem.FindSprite(gameObject.GameObjectId);
                            //    AddPropertyControls(componentObject, componentPanel, ChildObjectPanels);
                                break;
                            default:
                                MessageBox.Show(@$"Component type not implemented: {component}");
                                continue;
                        }
                    }
                }
                else if (list != null && list.Count > 0)
                {
                    foreach (var element in list)
                    {
                        if (element == null) continue;

                        int rowIndex = listPanel.RowCount;
                        listPanel.RowCount += 1;
                        listPanel.RowStyles.Add(new RowStyle(SizeType.AutoSize));

                        var childPanelView = new ObjectPanelView(levelEditorForm, element, ToolTip, this);
                        ChildObjectPanels.Add(childPanelView);
                        listPanel.Controls.Add(childPanelView, 0, rowIndex);

                        childPanelView.PropertyChanged += (s, e) => NotifyPropertyChanged();
                    }
                }

                ListModifier(listPanel, list, member, obj, elementType);
            }
            catch (Exception ex)
            {
                Console.WriteLine($"Error updating list panel: {ex.Message}");
            }
        }

        private void ListModifier(TableLayoutPanel listPanel, IList list, MemberInfo member, object obj, Type elementType)
        {
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

            if (list == (PanelObject as RenderPassLoaderModel)?.ClearValueList)
            {
                var renderPass = (RenderPassLoaderModel)PanelObject;
                int requiredClearValues = renderPass.RenderedTextureInfoModelList?.Count(x => x.AttachmentDescription?.LoadOp == VkAttachmentLoadOp.VK_ATTACHMENT_LOAD_OP_CLEAR) ?? 0;
                removeButton.Enabled = list.Count > requiredClearValues;
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

        public void NotifyPropertyChanged()
        {
            levelEditorForm.BeginInvoke((System.Windows.Forms.MethodInvoker)delegate
            {
                try
                {
                    UpdateObjectHierarchy(this);
                    AdjustPanelHeight();
                    levelEditorForm.QuickUpdateRenderPass();
                }
                catch (Exception ex)
                {
                    Console.WriteLine($"[NotifyPropertyChanged] {ex.Message}");
                }
            });
        }

        private void UpdateObjectHierarchy(ObjectPanelView objPanel)
        {
            if (objPanel._parentPanel == null)
            {
                return;
            }

            var objTypeName = objPanel.PanelObject.GetType().Name;
            var objectType = objPanel.PanelObject.GetType();
            var parentObjType = objPanel._parentPanel.PanelObject.GetType();

            var valueChanged = false;
            var properties = parentObjType.GetProperties(BindingFlags.Public | BindingFlags.Instance);
            var fields = parentObjType.GetFields(BindingFlags.Public | BindingFlags.Instance);
            var memberInfoList = properties.Cast<MemberInfo>().Concat(fields);
            foreach (var member in memberInfoList)
            {
                if (valueChanged)
                {
                    break;
                }

                object memberValue = null;
                if (member is PropertyInfo propInfo)
                {
                    memberValue = propInfo.GetValue(objPanel._parentPanel.PanelObject);
                }
                else if (member is FieldInfo fieldInfo)
                {
                    memberValue = fieldInfo.GetValue(objPanel._parentPanel.PanelObject);
                }

                if (memberValue != null)
                {
                    if (memberValue is string)
                    {
                        continue;
                    }

                    if (memberValue is IList && !(memberValue is string))
                    {
                        var listType = (IList)memberValue;
                        for (int x = 0; x < listType.Count; x++)
                        {
                            if (listType[x] != null)
                            {
                                if (listType[x].GetType().Name == objPanel.PanelObject?.GetType().Name &&
                                    listType[x].GetHashCode() == objPanel.PanelObject?.GetHashCode())

                                {
                                    valueChanged = true;
                                    listType[x] = objPanel.PanelObject;
                                    if (objPanel._parentPanel != objPanel)
                                    {
                                        UpdateObjectHierarchy(objPanel._parentPanel);
                                    }
                                    break;
                                }
                            }
                        }
                    }
                    else
                    {
                        if (memberValue.GetType().Name == objPanel.PanelObject?.GetType().Name &&
                            memberValue.GetHashCode() == objPanel.PanelObject?.GetHashCode())

                        {
                            if (member is PropertyInfo memberProp)
                            {
                                memberProp.SetValue(objPanel._parentPanel.PanelObject, objPanel.PanelObject);
                                UpdateObjectHierarchy(objPanel._parentPanel);
                            }
                            else if (member is FieldInfo memberField)
                            {
                                memberField.SetValue(objPanel._parentPanel.PanelObject, objPanel.PanelObject);
                                UpdateObjectHierarchy(objPanel._parentPanel);
                            }
                            break;
                        }

                    }
                }
            }
        }

        private T GetCustomAttribute<T>(MemberInfo member) where T : Attribute
        {
            if (DynamicControlPanelView._dynamicAttributes.TryGetValue(member, out var attributes))
            {
                return attributes.OfType<T>().FirstOrDefault();
            }
            return member.GetCustomAttribute<T>();
        }

        public void AddDynamicAttribute(MemberInfo member, Attribute attribute)
        {
            if (!DynamicControlPanelView._dynamicAttributes.ContainsKey(member))
            {
                DynamicControlPanelView._dynamicAttributes[member] = new List<Attribute>();
            }
            if (!DynamicControlPanelView._dynamicAttributes[member].Any(a => a.GetType() == attribute.GetType()))
            {
                DynamicControlPanelView._dynamicAttributes[member].Add(attribute);
            }
        }

        public void RemoveDynamicAttribute(MemberInfo member, Type attributeType)
        {
            if (DynamicControlPanelView._dynamicAttributes.ContainsKey(member))
            {
                DynamicControlPanelView._dynamicAttributes[member].RemoveAll(a => a.GetType() == attributeType);
                if (DynamicControlPanelView._dynamicAttributes[member].Count == 0)
                {
                    DynamicControlPanelView._dynamicAttributes.Remove(member);
                }
            }
        }

        private void RefreshProperties()
        {
            PopulateProperties();
            AdjustPanelHeight();
        }

        private void UpdateDepthStencilDependencies(VkPipelineDepthStencilStateCreateInfoModel depthStencil)
        {
            var depthWriteProp = typeof(VkPipelineDepthStencilStateCreateInfoModel).GetProperty(nameof(depthStencil.depthWriteEnable));
            var depthCompareProp = typeof(VkPipelineDepthStencilStateCreateInfoModel).GetProperty(nameof(depthStencil.depthCompareOp));
           // var stencilTestProp = typeof(VkPipelineDepthStencilStateCreateInfoModel).GetProperty(nameof(depthStencil.stencilTestEnable));
            var frontProp = typeof(VkPipelineDepthStencilStateCreateInfoModel).GetProperty(nameof(depthStencil.front));
            var backProp = typeof(VkPipelineDepthStencilStateCreateInfoModel).GetProperty(nameof(depthStencil.back));

            //if (!depthStencil.depthTestEnable == V)
            //{
            //    if (depthWriteProp != null) AddDynamicAttribute(depthWriteProp, new IgnorePropertyAttribute());
            //    if (depthCompareProp != null) AddDynamicAttribute(depthCompareProp, new IgnorePropertyAttribute());
            //}
            //else
            //{
            //    if (depthWriteProp != null) RemoveDynamicAttribute(depthWriteProp, typeof(IgnorePropertyAttribute));
            //    if (depthCompareProp != null) RemoveDynamicAttribute(depthCompareProp, typeof(IgnorePropertyAttribute));
            //}

            //if (!depthStencil.stencilTestEnable == 1)
            //{
            //    if (frontProp != null) AddDynamicAttribute(frontProp, new IgnorePropertyAttribute());
            //    if (backProp != null) AddDynamicAttribute(backProp, new IgnorePropertyAttribute());
            //}
            //else
            //{
            //    if (frontProp != null) RemoveDynamicAttribute(frontProp, typeof(IgnorePropertyAttribute));
            //    if (backProp != null) RemoveDynamicAttribute(backProp, typeof(IgnorePropertyAttribute));
            //}
        }

        private void UpdateBlendStateDependencies(VkPipelineColorBlendAttachmentState blendState)
        {
            var srcColorProp = typeof(PipelineColorBlendAttachmentState).GetProperty(nameof(blendState.srcColorBlendFactor));
            var dstColorProp = typeof(PipelineColorBlendAttachmentState).GetProperty(nameof(blendState.dstColorBlendFactor));
            var colorBlendOpProp = typeof(PipelineColorBlendAttachmentState).GetProperty(nameof(blendState.colorBlendOp));
            var srcAlphaProp = typeof(PipelineColorBlendAttachmentState).GetProperty(nameof(blendState.srcAlphaBlendFactor));
            var dstAlphaProp = typeof(PipelineColorBlendAttachmentState).GetProperty(nameof(blendState.dstAlphaBlendFactor));
            var alphaBlendOpProp = typeof(PipelineColorBlendAttachmentState).GetProperty(nameof(blendState.alphaBlendOp));

            //if (!blendState.blendEnable)
            //{
            //    if (srcColorProp != null) AddDynamicAttribute(srcColorProp, new IgnorePropertyAttribute());
            //    if (dstColorProp != null) AddDynamicAttribute(dstColorProp, new IgnorePropertyAttribute());
            //    if (colorBlendOpProp != null) AddDynamicAttribute(colorBlendOpProp, new IgnorePropertyAttribute());
            //    if (srcAlphaProp != null) AddDynamicAttribute(srcAlphaProp, new IgnorePropertyAttribute());
            //    if (dstAlphaProp != null) AddDynamicAttribute(dstAlphaProp, new IgnorePropertyAttribute());
            //    if (alphaBlendOpProp != null) AddDynamicAttribute(alphaBlendOpProp, new IgnorePropertyAttribute());
            //}
            //else
            //{
            //    if (srcColorProp != null) RemoveDynamicAttribute(srcColorProp, typeof(IgnorePropertyAttribute));
            //    if (dstColorProp != null) RemoveDynamicAttribute(dstColorProp, typeof(IgnorePropertyAttribute));
            //    if (colorBlendOpProp != null) RemoveDynamicAttribute(colorBlendOpProp, typeof(IgnorePropertyAttribute));
            //    if (srcAlphaProp != null) RemoveDynamicAttribute(srcAlphaProp, typeof(IgnorePropertyAttribute));
            //    if (dstAlphaProp != null) RemoveDynamicAttribute(dstAlphaProp, typeof(IgnorePropertyAttribute));
            //    if (alphaBlendOpProp != null) RemoveDynamicAttribute(alphaBlendOpProp, typeof(IgnorePropertyAttribute));
            //}
        }

        private void UpdateRenderAreaDependencies(RenderAreaModel renderArea)
        {
            var extentPanel = DynamicControlPanelView.ObjectPanelViewMap[renderArea];
            if (extentPanel != null)
            {
                var widthProp = typeof(Extent2D).GetProperty(nameof(Extent2D.Width));
                var heightProp = typeof(Extent2D).GetProperty(nameof(Extent2D.Height));

                if (renderArea.UseDefaultRenderArea)
                {
                    if (widthProp != null) extentPanel.AddDynamicAttribute(widthProp, new IgnorePropertyAttribute());
                    if (heightProp != null) extentPanel.AddDynamicAttribute(heightProp, new IgnorePropertyAttribute());
                }
                else
                {
                    if (widthProp != null) extentPanel.RemoveDynamicAttribute(widthProp, typeof(IgnorePropertyAttribute));
                    if (heightProp != null) extentPanel.RemoveDynamicAttribute(heightProp, typeof(IgnorePropertyAttribute));
                }
            }
        }
    }
}