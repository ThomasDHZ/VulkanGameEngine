using GlmSharp;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Linq;
using System.Reflection;
using System.Windows.Forms;
using VulkanGameEngineLevelEditor.GameEngineAPI;
using VulkanGameEngineLevelEditor.GameEngine.Systems;
using VulkanGameEngineLevelEditor.LevelEditor.Attributes;

namespace VulkanGameEngineLevelEditor.LevelEditor.EditorEnhancements
{
    public class DynamicControlPanelView : TableLayoutPanel
    {
        private const int RowHeight = 32;
        private const int MinimumPanelSize = 320;

        private object _targetObject;
        private TableLayoutPanel _contentPanel;
        private ToolTip _toolTip;
        private List<ObjectPanelView> _objectPanelViewList = new List<ObjectPanelView>();
        private List<UpdateProperty> _updatePropertiesList = new List<UpdateProperty>();

        public DynamicControlPanelView()
        {
            _toolTip = new ToolTip();
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
            this.BackColor = Color.FromArgb(40, 40, 40);

            _contentPanel = new TableLayoutPanel
            {
                Dock = DockStyle.Fill,
                AutoScroll = true,
                AutoSize = true,
                BackColor = Color.FromArgb(40, 40, 40),
                ForeColor = Color.White,
                ColumnCount = 1,
                ColumnStyles = { new ColumnStyle(SizeType.Percent, 100F) }
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
                    UpdatePanels();
                }
            }
        }

        private void UpdatePanels()
        {
            try
            {
                _contentPanel.Controls.Clear();
                if (_targetObject != null)
                {
                    var existingPanel = _objectPanelViewList.FirstOrDefault(panel => panel.PanelObject == _targetObject);
                    if (existingPanel == null)
                    {
                        existingPanel = new ObjectPanelView(_targetObject, _toolTip);
                        _objectPanelViewList.Add(existingPanel);
                    }
                    _contentPanel.Controls.Add(existingPanel);
                }
                AdjustContentHeight();
            }
            catch (Exception ex)
            {
                Console.WriteLine($"Error updating panels: {ex.Message}");
            }
        }

        public void CreatePropertyControl(object parentObject, object obj)
        {
            if (obj == null) return;

            try
            {
                var existingPanel = _objectPanelViewList.FirstOrDefault(panel => panel.PanelObject == obj);
                if (existingPanel == null)
                {
                    existingPanel = new ObjectPanelView(obj, _toolTip);
                    _objectPanelViewList.Add(existingPanel);
                    _contentPanel.Controls.Add(existingPanel);
                    AdjustContentHeight();
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine($"Error creating property control for {obj?.GetType().Name}: {ex.Message}");
            }
        }

        public static Control TypeOfComponentList(object parentObj, PropertyInfo prop, bool readOnly)
        {
            try
            {
                var gameObject = parentObj as GameObject;
                if (gameObject == null) return null;

                var comboBox = new ComboBox
                {
                    Dock = DockStyle.Fill,
                    DropDownStyle = ComboBoxStyle.DropDownList,
                    Enabled = !readOnly
                };
                comboBox.Items.AddRange(Enum.GetNames(typeof(ComponentTypeEnum)));
                comboBox.SelectedIndexChanged += (s, e) =>
                {
                    if (comboBox.SelectedItem == null) return;

                    var selectedComponent = (ComponentTypeEnum)Enum.Parse(typeof(ComponentTypeEnum), comboBox.SelectedItem.ToString());
                    switch (selectedComponent)
                    {
                        case ComponentTypeEnum.kTransform2DComponent:
                            if (GameObjectSystem.Transform2DComponentMap.TryGetValue(gameObject.GameObjectId, out var transform))
                            {
                                var panelView = comboBox.FindForm()?.Controls.OfType<DynamicControlPanelView>().FirstOrDefault();
                                panelView?.CreatePropertyControl(parentObj, transform);
                            }
                            break;
                        case ComponentTypeEnum.kInputComponent:
                            if (GameObjectSystem.InputComponentMap.TryGetValue(gameObject.GameObjectId, out var input))
                            {
                                var panelView = comboBox.FindForm()?.Controls.OfType<DynamicControlPanelView>().FirstOrDefault();
                                panelView?.CreatePropertyControl(parentObj, input);
                            }
                            break;
                        case ComponentTypeEnum.kSpriteComponent:
                            var sprite = SpriteSystem.FindSprite(gameObject.GameObjectId);
                            //if (sprite != null)
                            //{
                            //    var panelView = comboBox.FindForm()?.Controls.OfType<DynamicControlPanelView>().FirstOrDefault();
                            //    panelView?.CreatePropertyControl(parentObj, sprite);
                            //}
                            break;
                    }
                };
                return comboBox;
            }
            catch (Exception ex)
            {
                Console.WriteLine($"Error creating component list control: {ex.Message}");
                return null;
            }
        }

        private void AdjustContentHeight()
        {
            try
            {
                if (_contentPanel.Controls.Count > 0)
                {
                    int totalHeight = _contentPanel.Controls.Cast<Control>()
                        .Sum(c => c.Height + c.Margin.Top + c.Margin.Bottom);
                    _contentPanel.Height = totalHeight + _contentPanel.Padding.Vertical;
                }
                else
                {
                    _contentPanel.Height = MinimumPanelSize;
                }
                this.PerformLayout();
            }
            catch (Exception ex)
            {
                Console.WriteLine($"Error adjusting content height: {ex.Message}");
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
    }

    public struct UpdateProperty
    {
        public object ParentObj;
        public object Obj;
    }
}