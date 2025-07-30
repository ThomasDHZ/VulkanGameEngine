using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace VulkanGameEngineLevelEditor.LevelEditor.ControlSubForms
{
    public class TypeOfPictureBox : PropertyEditorForm
    {
        public Image renderPassImage { get; set; } = null;
        public TypeOfPictureBox(object obj, PropertyInfo property, int minimumPanelSize, bool readOnly) : base(obj, property, minimumPanelSize, readOnly) { }

        public override Control CreateControl()
        {
            var pictureBox = new PictureBox
            {
                Dock = DockStyle.Fill,
                MinimumSize = new Size(700, 500),
                BackColor = Color.FromArgb(60, 60, 60),
                BorderStyle = BorderStyle.FixedSingle,
                ForeColor = Color.White,
                Image = renderPassImage,
            };
            CreateBaseControl(pictureBox);
            return pictureBox;
        }
    }
}
