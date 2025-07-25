using System;
using System.Drawing;
using System.Reflection;
using System.Windows.Forms;

namespace VulkanGameEngineLevelEditor.LevelEditor.ControlSubForms
{
    public abstract class PropertyEditorForm
    {
        protected object _obj;
        protected PropertyInfo _property;
        protected int _minimumPanelSize;
        protected bool _readOnly;

        protected const int BufferHeight = 32;
        protected const int RowHeight = 70;

        protected PropertyEditorForm(object obj, PropertyInfo property, int minimumPanelSize, bool readOnly)
        {
            _obj = obj;
            _property = property;
            _minimumPanelSize = minimumPanelSize;
            _readOnly = readOnly;
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