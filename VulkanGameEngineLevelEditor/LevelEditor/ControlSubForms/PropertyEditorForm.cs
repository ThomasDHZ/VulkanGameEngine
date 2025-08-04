using AutoMapper.Execution;
using System;
using System.Drawing;
using System.Reflection;
using System.Windows.Forms;
using VulkanGameEngineLevelEditor.LevelEditor.EditorEnhancements;

namespace VulkanGameEngineLevelEditor.LevelEditor.ControlSubForms
{
    public abstract class PropertyEditorForm
    {
        protected const int BufferHeight = 32;
        protected const int RowHeight = 70;

        protected object _obj;
        protected MemberInfo _member;
        protected FieldInfo _field;
        protected int _minimumPanelSize;
        protected bool _readOnly;
        protected ObjectPanelView _rootPanel;

        protected object GetValue()
        {
            if (_member is PropertyInfo prop)
                return prop.GetValue(_obj);
            else if (_member is FieldInfo field)
                return field.GetValue(_obj);
            else
                throw new InvalidOperationException("Member is not PropertyInfo or FieldInfo");
        }

        protected void SetValue(object value)
        {
            if (_member is PropertyInfo prop)
                prop.SetValue(_obj, value);
            else if (_member is FieldInfo field)
                field.SetValue(_obj, value);
            else
                throw new InvalidOperationException("Member is not PropertyInfo or FieldInfo");
        }

        protected PropertyEditorForm(ObjectPanelView rootPanel, object obj, MemberInfo member, int minimumPanelSize, bool readOnly)
        {
            _obj = obj;
            _member = member;
            _minimumPanelSize = minimumPanelSize;
            _readOnly = readOnly;
            _rootPanel = rootPanel;
        }

        public abstract Control CreateControl();
        protected Control CreateBaseControl(Control control)
        {
            control.Dock = DockStyle.Fill;
            control.BackColor = Color.FromArgb(60, 60, 60);
            control.ForeColor = Color.White;
            control.MinimumSize = new Size(0, _minimumPanelSize);
            return control;
        }
    }
}