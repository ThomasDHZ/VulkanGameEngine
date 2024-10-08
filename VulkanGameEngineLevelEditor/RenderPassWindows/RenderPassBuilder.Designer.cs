﻿using System.Windows.Forms;

namespace VulkanGameEngineLevelEditor.RenderPassWindows
{
    partial class RenderPassBuilder
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.listBox1 = new System.Windows.Forms.ListBox();
            this.panel1 = new System.Windows.Forms.Panel();
            this.richTextBox1 = new System.Windows.Forms.RichTextBox();
            this.propertyGrid1 = new System.Windows.Forms.PropertyGrid();
            this.RenderPassBuilderDebug = new System.Windows.Forms.RichTextBox();
            this.panel2 = new System.Windows.Forms.Panel();
            this.BuildButton = new System.Windows.Forms.Button();
            this.panel1.SuspendLayout();
            this.panel2.SuspendLayout();
            this.SuspendLayout();
            // 
            // listBox1
            // 
            this.listBox1.FormattingEnabled = true;
            this.listBox1.ItemHeight = 20;
            this.listBox1.Location = new System.Drawing.Point(13, 13);
            this.listBox1.Name = "listBox1";
            this.listBox1.Size = new System.Drawing.Size(243, 864);
            this.listBox1.TabIndex = 0;
            this.listBox1.SelectedIndexChanged += new System.EventHandler(this.listBox1_SelectedIndexChanged);
            // 
            // panel1
            // 
            this.panel1.Controls.Add(this.richTextBox1);
            this.panel1.Controls.Add(this.propertyGrid1);
            this.panel1.Location = new System.Drawing.Point(1142, 12);
            this.panel1.Name = "panel1";
            this.panel1.Size = new System.Drawing.Size(527, 865);
            this.panel1.TabIndex = 1;
            // 
            // richTextBox1
            // 
            this.richTextBox1.Location = new System.Drawing.Point(-1038, 41);
            this.richTextBox1.Name = "richTextBox1";
            this.richTextBox1.Size = new System.Drawing.Size(2304, 842);
            this.richTextBox1.TabIndex = 2;
            this.richTextBox1.Text = "";
            // 
            // propertyGrid1
            // 
            this.propertyGrid1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.propertyGrid1.Font = new System.Drawing.Font("Microsoft Sans Serif", 7F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.propertyGrid1.Location = new System.Drawing.Point(0, 0);
            this.propertyGrid1.Name = "propertyGrid1";
            this.propertyGrid1.Size = new System.Drawing.Size(527, 865);
            this.propertyGrid1.TabIndex = 0;
            // 
            // RenderPassBuilderDebug
            // 
            this.RenderPassBuilderDebug.Dock = System.Windows.Forms.DockStyle.Fill;
            this.RenderPassBuilderDebug.Location = new System.Drawing.Point(0, 0);
            this.RenderPassBuilderDebug.Name = "RenderPassBuilderDebug";
            this.RenderPassBuilderDebug.Size = new System.Drawing.Size(1657, 207);
            this.RenderPassBuilderDebug.TabIndex = 2;
            this.RenderPassBuilderDebug.Text = "";
            // 
            // panel2
            // 
            this.panel2.Controls.Add(this.RenderPassBuilderDebug);
            this.panel2.Location = new System.Drawing.Point(12, 902);
            this.panel2.Name = "panel2";
            this.panel2.Size = new System.Drawing.Size(1657, 207);
            this.panel2.TabIndex = 3;
            // 
            // BuildButton
            // 
            this.BuildButton.Location = new System.Drawing.Point(1061, 850);
            this.BuildButton.Name = "BuildButton";
            this.BuildButton.Size = new System.Drawing.Size(75, 27);
            this.BuildButton.TabIndex = 3;
            this.BuildButton.Text = "Build";
            this.BuildButton.UseVisualStyleBackColor = true;
            this.BuildButton.Click += new System.EventHandler(this.BuildButton_Click);
            // 
            // RenderPassBuilder
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(9F, 20F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(1681, 1110);
            this.Controls.Add(this.BuildButton);
            this.Controls.Add(this.panel2);
            this.Controls.Add(this.panel1);
            this.Controls.Add(this.listBox1);
            this.Name = "RenderPassBuilder";
            this.Text = "RenderPassBuilder";
            this.panel1.ResumeLayout(false);
            this.panel2.ResumeLayout(false);
            this.ResumeLayout(false);

        }

        #endregion

        private ListBox listBox1;
        private Panel panel1;
        private PropertyGrid propertyGrid1;
        private RichTextBox richTextBox1;
        private RichTextBox RenderPassBuilderDebug;
        private Panel panel2;
        private Button BuildButton;
    }
}