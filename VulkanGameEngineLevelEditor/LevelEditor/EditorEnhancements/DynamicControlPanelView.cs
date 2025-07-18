using GlmSharp;
using Microsoft.CodeAnalysis.CSharp.Syntax;
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

        private const int MIN_PANEL_HEIGHT = 40;
        private const int BUFFER_HEIGHT = 10;

        public DynamicControlPanelView()
        {
            InitializeComponents();
        }

        private void InitializeComponents()
        {
            if (DesignMode) return;

            this.Dock = DockStyle.Top;
            this.AutoScroll = true;
            this.Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right;

            _contentPanel = new TableLayoutPanel
            {
                Dock = DockStyle.Fill,
                AutoScroll = true,
                BackColor = Color.FromArgb(40, 40, 40),
                ForeColor = Color.White,
                ColumnCount = 1,
                ColumnStyles = { new ColumnStyle(SizeType.Percent, 100F) },
                RowStyles = { new RowStyle(SizeType.AutoSize) }
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
                }
            }
        }

        public static void CreatePropertyControl(object parentObject, object obj)
        {
            if (obj == null) return;

            int rowIndex = _contentPanel.RowCount;
            _contentPanel.RowCount += 1;
            _contentPanel.RowStyles.Add(new RowStyle(SizeType.AutoSize));

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
                Text = obj.GetType().Name,
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
                if (ignoreAttr != null)
                {
                    continue;
                }

                var readOnlyAttr = prop.GetCustomAttributes(typeof(ReadOnlyAttribute), true).FirstOrDefault() as ReadOnlyAttribute;
                bool isReadOnly = readOnlyAttr?.IsReadOnly ?? false;

                int propRowIndex = propTable.RowCount;
                propTable.RowCount += 1;
                propTable.RowStyles.Add(new RowStyle(SizeType.AutoSize, MIN_PANEL_HEIGHT + BUFFER_HEIGHT));
                var labelPanel = new Panel
                {
                    Dock = DockStyle.Fill,
                    BackColor = Color.FromArgb(70, 70, 70),
                    Margin = new Padding(2),
                    MinimumSize = new Size(0, MIN_PANEL_HEIGHT + BUFFER_HEIGHT)
                };
                propTable.Controls.Add(labelPanel, 0, propRowIndex);

                Label label = new Label
                {
                    Text = prop.Name,
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
                    MinimumSize = new Size(0, MIN_PANEL_HEIGHT + BUFFER_HEIGHT)
                };
                propTable.Controls.Add(controlPanel, 1, propRowIndex);

                Control control = null;
                if (typeof(IList).IsAssignableFrom(prop.PropertyType))
                {
                    var list = prop.GetValue(obj) as IList;
                    if (list != null &&
                        list.Count > 0)
                    {
                        if (!typeof(IEnumerable<string>).IsAssignableFrom(prop.PropertyType))
                        {
                            foreach (var childObj in list)
                            {
                                if (childObj != null)
                                {
                                    CreatePropertyControl(obj, childObj);
                                }
                            }
                        }
                        else
                        {
                            control = TypeOfString(obj, prop, isReadOnly);
                        }
                    }
                }
                else if (prop.PropertyType == typeof(string))
                {
                    control = TypeOfString(obj, prop, isReadOnly);
                }
                else if (prop.PropertyType == typeof(int))
                {
                    control = TypeOfInt(obj, prop, isReadOnly);
                }
                else if (prop.PropertyType == typeof(uint))
                {
                    control = TypeOfUint(obj, prop, isReadOnly);
                }
                else if (prop.PropertyType == typeof(bool))
                {
                    control = TypeOfBool(obj, prop, isReadOnly);
                }
                else if (prop.PropertyType == typeof(Guid))
                {
                    control = TypeOfGuid(obj, prop, isReadOnly);
                }
                else if (prop.PropertyType == typeof(List<ComponentTypeEnum>))
                {
                    control = TypeOfComponentList(obj, prop, isReadOnly);
                }
                else if (prop.PropertyType == typeof(vec2))
                {
                    TypeOfVec2(parentObject, obj, prop, controlPanel, isReadOnly);
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

        public static Control TypeOfString(object obj, PropertyInfo property, bool readOnly)
        {
            var textBox = new TextBox
            {
                Dock = DockStyle.Fill,
                Text = property.GetValue(obj)?.ToString() ?? "",
                TextAlign = HorizontalAlignment.Left,
                BackColor = Color.FromArgb(60, 60, 60),
                ForeColor = Color.White,
                ReadOnly = readOnly,
                MinimumSize = new Size(0, MIN_PANEL_HEIGHT)
            };
            if (!readOnly)
            {
                textBox.TextChanged += (s, e) => property.SetValue(obj, ((TextBox)s).Text);
            }
            return textBox;
        }

        public static Control TypeOfGuid(object obj, PropertyInfo property, bool readOnly)
        {
            string guid = ((Guid)property.GetValue(obj)).ToString();
            var textBox = new TextBox
            {
                Dock = DockStyle.Fill,
                Text = guid ?? "",
                TextAlign = HorizontalAlignment.Left,
                BackColor = Color.FromArgb(60, 60, 60),
                ForeColor = Color.White,
                ReadOnly = true,
                MinimumSize = new Size(0, MIN_PANEL_HEIGHT)
            };
            return textBox;
        }

        public static Control TypeOfBool(object obj, PropertyInfo property, bool readOnly)
        {
            bool value = (bool)property.GetValue(obj);
            if (readOnly)
            {
                var labelDisplay = new Label
                {
                    Dock = DockStyle.Fill,
                    Text = value.ToString() ?? "N/A",
                    TextAlign = ContentAlignment.MiddleRight,
                    BackColor = Color.FromArgb(60, 60, 60),
                    BorderStyle = BorderStyle.FixedSingle,
                    ForeColor = Color.White,
                    MinimumSize = new Size(0, MIN_PANEL_HEIGHT)
                };
                return labelDisplay;
            }
            else
            {
                var checkBox = new CheckBox
                {
                    Dock = DockStyle.Fill,
                    Checked = value,
                    MinimumSize = new Size(0, MIN_PANEL_HEIGHT)
                };
                checkBox.CheckedChanged += (s, e) =>
                {
                    try
                    {
                        property.SetValue(obj, ((CheckBox)s).Checked);
                    }
                    catch (Exception ex)
                    {
                        Console.WriteLine($"Error setting {property.Name}: {ex.Message}");
                    }
                };
                return checkBox;
            }
        }

        public static Control TypeOfInt(object obj, PropertyInfo property, bool readOnly)
        {
            int value = (int)property.GetValue(obj);
            if (readOnly)
            {
                var labelDisplay = new Label
                {
                    Dock = DockStyle.Fill,
                    Text = value.ToString() ?? "N/A",
                    TextAlign = ContentAlignment.MiddleRight,
                    BackColor = Color.FromArgb(60, 60, 60),
                    BorderStyle = BorderStyle.FixedSingle,
                    ForeColor = Color.White,
                    MinimumSize = new Size(0, MIN_PANEL_HEIGHT)
                };
                return labelDisplay;
            }
            else
            {
                var numeric = new NumericUpDown
                {
                    Dock = DockStyle.Fill,
                    Minimum = (decimal)int.MinValue,
                    Maximum = (decimal)int.MaxValue,
                    Value = (decimal)Math.Max(int.MinValue, Math.Min(int.MaxValue, value)),
                    MinimumSize = new Size(0, MIN_PANEL_HEIGHT)
                };
                numeric.ValueChanged += (s, e) =>
                {
                    try
                    {
                        property.SetValue(obj, (int)((NumericUpDown)s).Value);
                    }
                    catch (Exception ex)
                    {
                        Console.WriteLine($"Error setting {property.Name}: {ex.Message}");
                    }
                };
                return numeric;
            }
        }

        public static Control TypeOfUint(object obj, PropertyInfo property, bool readOnly)
        {
            uint value = (uint)property.GetValue(obj);
            if (readOnly)
            {
                var labelDisplay = new Label
                {
                    Dock = DockStyle.Fill,
                    Text = value.ToString() ?? "N/A",
                    TextAlign = ContentAlignment.MiddleRight,
                    BackColor = Color.FromArgb(60, 60, 60),
                    BorderStyle = BorderStyle.FixedSingle,
                    ForeColor = Color.White,
                    MinimumSize = new Size(0, MIN_PANEL_HEIGHT)
                };
                return labelDisplay;
            }
            else
            {
                var numeric = new NumericUpDown
                {
                    Dock = DockStyle.Fill,
                    Minimum = (decimal)0,
                    Maximum = (decimal)uint.MaxValue,
                    Value = (decimal)value,
                    MinimumSize = new Size(0, MIN_PANEL_HEIGHT)
                };
                numeric.ValueChanged += (s, e) =>
                {
                    try
                    {
                        property.SetValue(obj, (uint)((NumericUpDown)s).Value);
                    }
                    catch (Exception ex)
                    {
                        Console.WriteLine($"Error setting {property.Name}: {ex.Message}");
                    }
                };
                return numeric;
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

        public static void TypeOfVec2(object parentObject, object obj, PropertyInfo property, Panel controlPanel, bool readOnly)
        {
            var vec2Value = (vec2)property.GetValue(obj);
            int rowIndex = 0;

            var vec2Panel = new TableLayoutPanel
            {
                Dock = DockStyle.Fill,
                AutoScroll = true,
                BackColor = Color.FromArgb(70, 70, 70),
                ColumnCount = 2,
                ColumnStyles =
                {
                    new ColumnStyle(SizeType.Percent, 50F),
                    new ColumnStyle(SizeType.Percent, 50F)
                },
                RowStyles = { new RowStyle(SizeType.AutoSize, MIN_PANEL_HEIGHT + BUFFER_HEIGHT) }
            };
            controlPanel.Controls.Add(vec2Panel);

            var xLabelPanel = AddPanel();
            var xLabel = new Label { Text = "X", Dock = DockStyle.Fill, TextAlign = ContentAlignment.MiddleLeft, ForeColor = Color.White, Margin = new Padding(5) };
            xLabelPanel.Controls.Add(xLabel);
            vec2Panel.Controls.Add(xLabelPanel, 0, rowIndex);

            var xControlPanel = AddPanel();
            var numericX = new NumericUpDown
            {
                Dock = DockStyle.Fill,
                Minimum = decimal.MinValue,
                Maximum = decimal.MaxValue,
                BackColor = Color.FromArgb(60, 60, 60),
                ForeColor = Color.White,
                Value = (decimal)vec2Value.x,
                MinimumSize = new Size(0, MIN_PANEL_HEIGHT),
                Margin = new Padding(5)
            };
            xControlPanel.Controls.Add(numericX);
            vec2Panel.Controls.Add(xControlPanel, 1, rowIndex);

            rowIndex++;
            vec2Panel.RowCount += 1;
            vec2Panel.RowStyles.Add(new RowStyle(SizeType.AutoSize, MIN_PANEL_HEIGHT + BUFFER_HEIGHT));

            var yLabelPanel = AddPanel();
            var yLabel = new Label { Text = "Y", Dock = DockStyle.Fill, TextAlign = ContentAlignment.MiddleLeft, ForeColor = Color.White, Margin = new Padding(5) };
            yLabelPanel.Controls.Add(yLabel);
            vec2Panel.Controls.Add(yLabelPanel, 0, rowIndex);

            var yControlPanel = AddPanel();
            var numericY = new NumericUpDown
            {
                Dock = DockStyle.Fill,
                Minimum = decimal.MinValue,
                Maximum = decimal.MaxValue,
                BackColor = Color.FromArgb(60, 60, 60),
                ForeColor = Color.White,
                Value = (decimal)vec2Value.y,
                MinimumSize = new Size(0, MIN_PANEL_HEIGHT),
                Margin = new Padding(5)
            };
            yControlPanel.Controls.Add(numericY);
            vec2Panel.Controls.Add(yControlPanel, 1, rowIndex);

            numericX.ValueChanged += (s, e) =>
            {
                var newX = (float)((NumericUpDown)s).Value;
                var newVec2 = new vec2(newX, vec2Value.y);
                property.SetValue(obj, newVec2);
                UpdatePropertiesList.Add(new UpdateProperty { ParentObj = parentObject, Obj = obj });
            };

            numericY.ValueChanged += (s, e) =>
            {
                var newY = (float)((NumericUpDown)s).Value;
                var newVec2 = new vec2(vec2Value.x, newY);
                property.SetValue(obj, newVec2);
                UpdatePropertiesList.Add(new UpdateProperty { ParentObj = parentObject, Obj = obj });
            };
        }

        protected override void OnResize(EventArgs e)
        {
            base.OnResize(e);
            if (!DesignMode && _targetObject != null)
            {
                _contentPanel.Controls.Clear();
                _contentPanel.RowStyles.Clear();
                _contentPanel.RowCount = 0;
                CreatePropertyControl(null, _targetObject);
            }
        }

        private static Panel AddPanel()
        {
            return new Panel
            {
                Dock = DockStyle.Fill,
                BackColor = Color.FromArgb(60, 60, 60),
                Margin = new Padding(2),
                MinimumSize = new Size(0, MIN_PANEL_HEIGHT + BUFFER_HEIGHT)
            };
        }
    }
}