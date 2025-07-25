using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using VulkanGameEngineLevelEditor.LevelEditor.ControlSubForms;

namespace VulkanGameEngineLevelEditor.LevelEditor.Dialog
{
    public class TypeOfUintForm : PropertyEditorForm
    {
        public TypeOfUintForm(object obj, PropertyInfo property, int minimumPanelSize, bool readOnly) : base(obj, property, minimumPanelSize, readOnly) { }

        public override Control CreateControl()
        {
            uint value = (uint)_property.GetValue(_obj);
            if (_readOnly)
            {
                var labelDisplay = new Label
                {
                    Dock = DockStyle.Fill,
                    Text = value.ToString() ?? "N/A",
                    TextAlign = ContentAlignment.MiddleRight,
                    BackColor = Color.FromArgb(60, 60, 60),
                    BorderStyle = BorderStyle.FixedSingle,
                    ForeColor = Color.White,
                    MinimumSize = new Size(0, _minimumPanelSize)
                };
                CreateBaseControl(labelDisplay);
                return labelDisplay;
            }
            else
            {
                var numeric = new NumericUpDown
                {
                    Dock = DockStyle.Fill,
                    Minimum = (decimal)0,
                    Maximum = (decimal)int.MaxValue,
                    Value = (decimal)Math.Max(int.MinValue, Math.Min(int.MaxValue, value)),
                    MinimumSize = new Size(0, _minimumPanelSize)
                };
                numeric.ValueChanged += (s, e) =>
                {
                    try
                    {
                        _property.SetValue(_obj, (int)((NumericUpDown)s).Value);
                    }
                    catch (Exception ex)
                    {
                        Console.WriteLine($"Error setting {_property.Name}: {ex.Message}");
                    }
                };
                CreateBaseControl(numeric);
                return numeric;
            }
        }
    }
}
