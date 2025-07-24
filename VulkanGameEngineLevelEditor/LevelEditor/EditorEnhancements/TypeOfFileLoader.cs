using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Security.Cryptography;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using VulkanGameEngineLevelEditor.LevelEditor.Dialog;
using static System.Windows.Forms.VisualStyles.VisualStyleElement;

namespace VulkanGameEngineLevelEditor.LevelEditor.EditorEnhancements
{
    public class TypeOfFileLoader : UserControl
    {
        private System.Windows.Forms.Button button1;
        private System.Windows.Forms.TextBox textBox1;
        private OpenFileDialog openFileDialog1;
        private string StringFilter = string.Empty;
        public TypeOfFileLoader(string stringFilter = "All Files (*.*)|*.*")
        {
            StringFilter = stringFilter;
            InitializeComponent();
        }

        private void InitializeComponent()
        {
            button1 = new System.Windows.Forms.Button();
            textBox1 = new System.Windows.Forms.TextBox();
            openFileDialog1 = new OpenFileDialog();
            SuspendLayout();
            // 
            // button1
            // 
            button1.Location = new System.Drawing.Point(159, 0);
            button1.Name = "button1";
            button1.Size = new System.Drawing.Size(112, 34);
            button1.TabIndex = 0;
            button1.Text = "Load";
            button1.UseVisualStyleBackColor = true;
            button1.Click += button1_Click;
            // 
            // textBox1
            // 
            textBox1.Location = new System.Drawing.Point(3, 3);
            textBox1.Name = "textBox1";
            textBox1.Size = new System.Drawing.Size(150, 31);
            textBox1.TabIndex = 1;
            // 
            // openFileDialog1
            // 
            openFileDialog1.FileName = "openFileDialog1";
            openFileDialog1.Filter = StringFilter;
            openFileDialog1.FilterIndex = 1;
            openFileDialog1.FileOk += openFileDialog1_FileOk;
            // 
            // TypeOfFileLoader
            // 
            Controls.Add(textBox1);
            Controls.Add(button1);
            Name = "TypeOfFileLoader";
            Size = new System.Drawing.Size(276, 34);
            ResumeLayout(false);
            PerformLayout();

        }

        private void openFileDialog1_FileOk(object sender, System.ComponentModel.CancelEventArgs e)
        {
            //var dialog = sender as OpenFileDialog;
            //string fileExtension = System.IO.Path.GetExtension(dialog.FileName).ToLower();

            //if (!allowedExtensions.Contains(fileExtension))
            //{
            //    MessageBox.Show($"Please select a valid shader file: {string.Join(", ", allowedExtensions)}");
            //    e.Cancel = true; // Cancel the dialog's OK to prevent further processing
            //}
        }

        private void button1_Click(object sender, EventArgs e)
        {
            using (OpenFileDialog openFileDialog = new OpenFileDialog())
            {
                openFileDialog.Filter = "Shaders (*.spv, *.vert, *.frag)|*.spv;*.vert;*.frag|All Files (*.*)|*.*";
                openFileDialog.FilterIndex = 1;
                if (openFileDialog.ShowDialog() == DialogResult.OK)
                {
                    textBox1.Text = openFileDialog.FileName;
                }
            }
        }
    }
}
