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
            if (_obj is DynamicComponentWrapper wrapper) return wrapper.GetMemberValue(_member);
            if (_member is PropertyInfo p) return p.GetValue(_obj);
            if (_member is FieldInfo f) return f.GetValue(_obj);
            throw new InvalidOperationException();
        }

        protected void SetValue(object value)
        {
            if (_obj is DynamicComponentWrapper wrapper)
            {
                wrapper.SetMemberValue(_member, value);
                return;
            }

            if (_member is PropertyInfo p) p.SetValue(_obj, value);
            else if (_member is FieldInfo f) f.SetValue(_obj, value);
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