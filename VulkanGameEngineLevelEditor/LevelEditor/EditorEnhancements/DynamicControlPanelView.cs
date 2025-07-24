using GlmSharp;
using System;
using System.Collections;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Linq;
using System.Reflection;
using System.Windows.Forms;
using VulkanGameEngineLevelEditor.GameEngine.Systems;
using VulkanGameEngineLevelEditor.GameEngineAPI;
using VulkanGameEngineLevelEditor.LevelEditor.Attributes;
using VulkanGameEngineLevelEditor.LevelEditor.Dialog;

namespace VulkanGameEngineLevelEditor.LevelEditor.EditorEnhancements
{
    public struct UpdateProperty
    {
        public object ParentObj;
        public object Obj;
    }

    [DesignerCategory("")]
    public class DynamicControlPanelView : TableLayoutPanel
    {
        private static object _targetObject;
        private static TableLayoutPanel _contentPanel;
        public static List<UpdateProperty> UpdatePropertiesList = new List<UpdateProperty>();

        private const int BufferHeight = 32;
        private const int MinimumPanelSize = 100;
        private const int RowHeight = 70;

        public DynamicControlPanelView()
        {
            InitializeComponents();
        }

        private void InitializeComponents()
        {
            if (DesignMode) return;

            this.Dock = DockStyle.Right;
            this.AutoScroll = true;
            this.AutoSize = false;
            this.Anchor = AnchorStyles.Top | AnchorStyles.Right | AnchorStyles.Bottom;
            this.MinimumSize = new Size(100, 0);

            _contentPanel = new TableLayoutPanel
            {
                Dock = DockStyle.Fill,
                AutoScroll = true,
                BackColor = Color.FromArgb(40, 40, 40),
                ForeColor = Color.White,
                ColumnCount = 1,
                ColumnStyles = { new ColumnStyle(SizeType.Percent, 100F) },
                AutoSize = false
            };
            this.Controls.Add(_contentPanel);
        }

        public object SelectedObject
        {
            get => _targetObject;
            set
            {
                if (DesignMode) return;
                if (_targetObject != value)
                {
                    _targetObject = value;
                    _contentPanel.Controls.Clear();
                    _contentPanel.RowStyles.Clear();
                    _contentPanel.RowCount = 0;

                    if (_targetObject != null)
                    {
                        CreatePropertyControl(null, _targetObject);
                        var listProp = _targetObject.GetType().GetProperties()
                            .FirstOrDefault(p => typeof(IList).IsAssignableFrom(p.PropertyType));
                        if (listProp != null)
                        {
                            var list = listProp.GetValue(_targetObject) as IList;
                            if (list != null)
                            {
                                foreach (var item in list)
                                {
                                    if (item != null)
                                    {
                                        CreatePropertyControl(_targetObject, item);
                                    }
                                }
                            }
                        }
                    }
                    AdjustContentHeight();
                }
            }
        }

        public static void CreatePropertyControl(object parentObject, object obj)
        {
            if (obj == null) return;

            int rowIndex = _contentPanel.RowCount;
            _contentPanel.RowCount += 1;
            _contentPanel.RowStyles.Add(new RowStyle(SizeType.Absolute, RowHeight));

            var panelDisplayNameAttr = obj.GetType().GetCustomAttribute<DisplayNameAttribute>();
            string panelDisplayName = panelDisplayNameAttr?.DisplayName ?? obj.GetType().Name;

            var objectPanel = new Panel
            {
                Dock = DockStyle.Top,
                AutoSize = true,
                BackColor = Color.FromArgb(0, 0, 60),
                BorderStyle = BorderStyle.FixedSingle,
                Padding = new Padding(5)
            };
            _contentPanel.Controls.Add(objectPanel, 0, rowIndex);
            _contentPanel.SetRowSpan(objectPanel, 1);

            var headerPanel = new Panel
            {
                Dock = DockStyle.Top,
                BackColor = Color.FromArgb(0, 0, 60),
                Height = 30
            };
            objectPanel.Controls.Add(headerPanel);

            Label objLabel = new Label
            {
                Text = panelDisplayName,
                Dock = DockStyle.Fill,
                TextAlign = ContentAlignment.MiddleLeft,
                ForeColor = Color.White,
                Margin = new Padding(5)
            };
            headerPanel.Controls.Add(objLabel);

            var propTable = new TableLayoutPanel
            {
                Dock = DockStyle.Fill,
                AutoSize = true,
                AutoScroll = true,
                BackColor = Color.FromArgb(90, 90, 90),
                ColumnCount = 2,
                ColumnStyles =
                {
                    new ColumnStyle(SizeType.Percent, 30F),
                    new ColumnStyle(SizeType.Percent, 70F)
                },
                Padding = new Padding(0, 35, 0, 0)
            };
            objectPanel.Controls.Add(propTable);

            foreach (var prop in obj.GetType().GetProperties())
            {
                var ignoreAttr = prop.GetCustomAttributes(typeof(IgnorePropertyAttribute), true).FirstOrDefault() as IgnorePropertyAttribute;
                if (ignoreAttr != null) continue;

                var propertyDisplayNameAttr = prop.GetCustomAttribute<DisplayNameAttribute>();
                string propertyDisplayName = propertyDisplayNameAttr?.DisplayName ?? prop.Name;

                var readOnlyAttr = prop.GetCustomAttributes(typeof(ReadOnlyAttribute), true).FirstOrDefault() as ReadOnlyAttribute;
                bool isReadOnly = readOnlyAttr?.IsReadOnly ?? false;

                var controlTypeAttr = prop.GetCustomAttributes(typeof(ControlTypeAttribute), true).FirstOrDefault() as ControlTypeAttribute;
                
                int propRowIndex = propTable.RowCount;
                propTable.RowCount += 1;
                propTable.RowStyles.Add(new RowStyle(SizeType.AutoSize, RowHeight));

                var labelPanel = new Panel
                {
                    Dock = DockStyle.Fill,
                    BackColor = Color.FromArgb(70, 70, 70),
                    Margin = new Padding(2),
                    Height = RowHeight
                };
                propTable.Controls.Add(labelPanel, 0, propRowIndex);

                Label label = new Label
                {
                    Text = propertyDisplayName,
                    Dock = DockStyle.Fill,
                    TextAlign = ContentAlignment.MiddleLeft,
                    ForeColor = Color.White,
                    Margin = new Padding(5)
                };
                labelPanel.Controls.Add(label);

                var controlPanel = new Panel
                {
                    Dock = DockStyle.Fill,
                    BackColor = Color.FromArgb(70, 70, 70),
                    Margin = new Padding(2),
                    Height = RowHeight
                };
                propTable.Controls.Add(controlPanel, 1, propRowIndex);

                Control control = null;
                if (controlTypeAttr != null)
                {
                    if (controlTypeAttr.ControlType == typeof(TypeOfFileLoader))
                    {
                        control = new TypeOfFileLoader("Shader Files (*.spv, *.vert, *.frag)|*.spv;*.vert;*.frag|All Files (*.*)|*.*");
                    }
                }
                else if (typeof(IList).IsAssignableFrom(prop.PropertyType))
                {
                    var list = prop.GetValue(obj) as IList;
                    if (list != null && list.Count > 0)
                    {
                        if (!typeof(IEnumerable<string>).IsAssignableFrom(prop.PropertyType))
                        {
                            foreach (var childObj in list)
                            {
                                if (childObj != null) CreatePropertyControl(obj, childObj);
                            }
                        }
                        else
                        {

                            control = new TypeOfStringForm(obj, prop, RowHeight, isReadOnly).CreateControl();
                        }
                    }
                }
                else if (prop.PropertyType == typeof(string))
                {
                    control = new TypeOfStringForm(obj, prop, RowHeight, isReadOnly).CreateControl();
                }
                else if (prop.PropertyType == typeof(int))
                {
                    control = new TypeOfIntForm(obj, prop, RowHeight, isReadOnly).CreateControl();
                }
                else if (prop.PropertyType == typeof(uint))
                {
                    control = new TypeOfUintForm(obj, prop, RowHeight, isReadOnly).CreateControl();
                }
                else if (prop.PropertyType == typeof(bool))
                {
                    control = new TypeOfBool(obj, prop, RowHeight, isReadOnly).CreateControl();
                }
                else if (prop.PropertyType == typeof(Guid))
                {
                    control = new TypeOfGuidForm(obj, prop, RowHeight, isReadOnly).CreateControl();
                }
                else if (prop.PropertyType == typeof(List<ComponentTypeEnum>))
                {
                    control = TypeOfComponentList(obj, prop, isReadOnly);
                }
                else if (prop.PropertyType == typeof(vec2))
                {
                    new TypeOfVec2Form(obj, prop, RowHeight, isReadOnly).CreateControl();
                    control = null;
                }

                if (control != null)
                {
                    control.Dock = DockStyle.Fill;
                    control.Margin = new Padding(5);
                    controlPanel.Controls.Add(control);
                }
            }
        }

        public static Control TypeOfComponentList(object parentObj, PropertyInfo prop, bool readOnly)
        {
            var gameObject = parentObj as GameObject;
            if (gameObject != null)
            {
                var componentList = gameObject.GameObjectComponentTypeList;
                foreach (var component in componentList)
                {
                    switch (component)
                    {
                        case ComponentTypeEnum.kTransform2DComponent:
                            {
                                if (GameObjectSystem.Transform2DComponentMap.TryGetValue(gameObject.GameObjectId, out var transformComponent))
                                {
                                    CreatePropertyControl(parentObj, transformComponent);
                                }
                                break;
                            }
                        case ComponentTypeEnum.kInputComponent:
                            {
                                if (GameObjectSystem.InputComponentMap.TryGetValue(gameObject.GameObjectId, out var inputComponent))
                                {
                                    CreatePropertyControl(parentObj, inputComponent);
                                }
                                break;
                            }
                        case ComponentTypeEnum.kSpriteComponent:
                            {
                                var spriteComponent = SpriteSystem.FindSprite(gameObject.GameObjectId);
                                CreatePropertyControl(parentObj, spriteComponent);
                                break;
                            }
                    }
                }
            }
            return null;
        }

        private void AdjustContentHeight()
        {
            if (_contentPanel.Controls.Count > 0)
            {
                int totalHeight = _contentPanel.Controls.Cast<Control>().Sum(c => c.Height + c.Margin.Vertical);
                _contentPanel.Height = totalHeight;
            }
            else
            {
                _contentPanel.Height = 0;
            }
        }

        protected override void OnResize(EventArgs e)
        {
            base.OnResize(e);
            if (!DesignMode && _contentPanel != null)
            {
                _contentPanel.Width = this.ClientSize.Width;
                AdjustContentHeight();
            }
        }

        private static Panel AddPanel()
        {
            return new Panel
            {
                Dock = DockStyle.Fill,
                BackColor = Color.FromArgb(60, 60, 60),
                Margin = new Padding(2),
                Height = RowHeight
            };
        }
    }
}