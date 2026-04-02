using System.Windows.Forms;

namespace VulkanGameEngineLevelEditor.LevelEditor.Forms
{
    partial class LevelEditorForm
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
            components = new System.ComponentModel.Container();
            tabControl1 = new TabControl();
            tabPage1 = new TabPage();
            VulkanLoggerBox = new RichTextBox();
            tabPage2 = new TabPage();
            GameObjectListView = new ListView();
            imageList1 = new ImageList(components);
            tabPage3 = new TabPage();
            MaterialListView = new ListView();
            tabPage4 = new TabPage();
            TextureListView = new ListView();
            panel1 = new Panel();
            RenderBox = new PictureBox();
            LevelEditorMenuStrip = new MenuStrip();
            fileToolStripMenuItem = new ToolStripMenuItem();
            buildToolStripMenuItem = new ToolStripMenuItem();
            treeView1 = new TreeView();
            toolStrip1 = new ToolStrip();
            openFileDialog1 = new OpenFileDialog();
            tabControl1.SuspendLayout();
            tabPage1.SuspendLayout();
            tabPage2.SuspendLayout();
            tabPage3.SuspendLayout();
            tabPage4.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)RenderBox).BeginInit();
            LevelEditorMenuStrip.SuspendLayout();
            SuspendLayout();
            // 
            // tabControl1
            // 
            tabControl1.Controls.Add(tabPage1);
            tabControl1.Controls.Add(tabPage2);
            tabControl1.Controls.Add(tabPage3);
            tabControl1.Controls.Add(tabPage4);
            tabControl1.Dock = DockStyle.Bottom;
            tabControl1.Location = new System.Drawing.Point(0, 754);
            tabControl1.Name = "tabControl1";
            tabControl1.SelectedIndex = 0;
            tabControl1.Size = new System.Drawing.Size(1898, 270);
            tabControl1.TabIndex = 0;
            // 
            // tabPage1
            // 
            tabPage1.BackColor = System.Drawing.Color.FromArgb(40, 40, 40);
            tabPage1.BorderStyle = BorderStyle.Fixed3D;
            tabPage1.Controls.Add(VulkanLoggerBox);
            tabPage1.Location = new System.Drawing.Point(4, 34);
            tabPage1.Name = "tabPage1";
            tabPage1.Padding = new Padding(3);
            tabPage1.Size = new System.Drawing.Size(1890, 232);
            tabPage1.TabIndex = 0;
            tabPage1.Text = "Vulkan Logger";
            // 
            // VulkanLoggerBox
            // 
            VulkanLoggerBox.BackColor = System.Drawing.Color.FromArgb(40, 40, 40);
            VulkanLoggerBox.Dock = DockStyle.Fill;
            VulkanLoggerBox.ForeColor = System.Drawing.SystemColors.Window;
            VulkanLoggerBox.Location = new System.Drawing.Point(3, 3);
            VulkanLoggerBox.Name = "VulkanLoggerBox";
            VulkanLoggerBox.Size = new System.Drawing.Size(1880, 222);
            VulkanLoggerBox.TabIndex = 0;
            VulkanLoggerBox.Text = "";
            // 
            // tabPage2
            // 
            tabPage2.Controls.Add(GameObjectListView);
            tabPage2.Location = new System.Drawing.Point(4, 34);
            tabPage2.Name = "tabPage2";
            tabPage2.Padding = new Padding(3);
            tabPage2.Size = new System.Drawing.Size(1890, 232);
            tabPage2.TabIndex = 1;
            tabPage2.Text = "GameObjects";
            tabPage2.UseVisualStyleBackColor = true;
            // 
            // GameObjectListView
            // 
            GameObjectListView.BackColor = System.Drawing.Color.FromArgb(40, 40, 40);
            GameObjectListView.Dock = DockStyle.Fill;
            GameObjectListView.ForeColor = System.Drawing.Color.White;
            GameObjectListView.LargeImageList = imageList1;
            GameObjectListView.Location = new System.Drawing.Point(3, 3);
            GameObjectListView.Name = "GameObjectListView";
            GameObjectListView.Size = new System.Drawing.Size(1884, 226);
            GameObjectListView.TabIndex = 0;
            GameObjectListView.UseCompatibleStateImageBehavior = false;
            GameObjectListView.ItemDrag += GameObjectListView_ItemDrag;
            // 
            // imageList1
            // 
            imageList1.ColorDepth = ColorDepth.Depth32Bit;
            imageList1.ImageSize = new System.Drawing.Size(64, 64);
            imageList1.TransparentColor = System.Drawing.Color.Maroon;
            // 
            // tabPage3
            // 
            tabPage3.BackColor = System.Drawing.Color.FromArgb(60, 60, 60);
            tabPage3.Controls.Add(MaterialListView);
            tabPage3.Location = new System.Drawing.Point(4, 34);
            tabPage3.Name = "tabPage3";
            tabPage3.Padding = new Padding(3);
            tabPage3.Size = new System.Drawing.Size(1890, 232);
            tabPage3.TabIndex = 2;
            tabPage3.Text = "Materials";
            tabPage3.UseVisualStyleBackColor = true;
            // 
            // MaterialListView
            // 
            MaterialListView.BackColor = System.Drawing.Color.FromArgb(40, 40, 40);
            MaterialListView.Dock = DockStyle.Fill;
            MaterialListView.ForeColor = System.Drawing.SystemColors.Window;
            MaterialListView.Location = new System.Drawing.Point(3, 3);
            MaterialListView.Name = "MaterialListView";
            MaterialListView.Size = new System.Drawing.Size(1884, 226);
            MaterialListView.TabIndex = 0;
            MaterialListView.UseCompatibleStateImageBehavior = false;
            // 
            // tabPage4
            // 
            tabPage4.Controls.Add(TextureListView);
            tabPage4.Location = new System.Drawing.Point(4, 34);
            tabPage4.Name = "tabPage4";
            tabPage4.Padding = new Padding(3);
            tabPage4.Size = new System.Drawing.Size(1890, 232);
            tabPage4.TabIndex = 3;
            tabPage4.Text = "Textures";
            tabPage4.UseVisualStyleBackColor = true;
            // 
            // TextureListView
            // 
            TextureListView.BackColor = System.Drawing.Color.FromArgb(40, 40, 40);
            TextureListView.Dock = DockStyle.Fill;
            TextureListView.Location = new System.Drawing.Point(3, 3);
            TextureListView.Name = "TextureListView";
            TextureListView.Size = new System.Drawing.Size(1884, 226);
            TextureListView.TabIndex = 0;
            TextureListView.UseCompatibleStateImageBehavior = false;
            // 
            // panel1
            // 
            panel1.BackColor = System.Drawing.Color.FromArgb(40, 40, 40);
            panel1.BorderStyle = BorderStyle.Fixed3D;
            panel1.Dock = DockStyle.Right;
            panel1.Location = new System.Drawing.Point(1578, 33);
            panel1.Name = "panel1";
            panel1.Size = new System.Drawing.Size(320, 721);
            panel1.TabIndex = 3;
            // 
            // RenderBox
            // 
            RenderBox.AllowDrop = true;
            RenderBox.BackColor = System.Drawing.Color.FromArgb(40, 40, 40);
            RenderBox.BorderStyle = BorderStyle.Fixed3D;
            RenderBox.Dock = DockStyle.Fill;
            RenderBox.Location = new System.Drawing.Point(320, 33);
            RenderBox.Name = "RenderBox";
            RenderBox.Size = new System.Drawing.Size(1258, 721);
            RenderBox.TabIndex = 4;
            RenderBox.TabStop = false;
            RenderBox.DragDrop += RenderBox_DragDrop;
            RenderBox.DragEnter += RenderBox_DragEnter;
            RenderBox.MouseDown += RendererBox_MouseDown;
            RenderBox.MouseMove += RendererBox_MouseMove;
            RenderBox.MouseUp += RendererBox_MouseUp;
            RenderBox.MouseWheel += RendererBox_MouseWheel;
            // 
            // LevelEditorMenuStrip
            // 
            LevelEditorMenuStrip.BackColor = System.Drawing.Color.FromArgb(40, 40, 48);
            LevelEditorMenuStrip.GripStyle = ToolStripGripStyle.Visible;
            LevelEditorMenuStrip.ImageScalingSize = new System.Drawing.Size(24, 24);
            LevelEditorMenuStrip.Items.AddRange(new ToolStripItem[] { fileToolStripMenuItem, buildToolStripMenuItem });
            LevelEditorMenuStrip.Location = new System.Drawing.Point(0, 0);
            LevelEditorMenuStrip.Name = "LevelEditorMenuStrip";
            LevelEditorMenuStrip.Size = new System.Drawing.Size(1898, 33);
            LevelEditorMenuStrip.TabIndex = 5;
            LevelEditorMenuStrip.Text = "menuStrip1";
            // 
            // fileToolStripMenuItem
            // 
            fileToolStripMenuItem.ForeColor = System.Drawing.Color.White;
            fileToolStripMenuItem.Name = "fileToolStripMenuItem";
            fileToolStripMenuItem.Size = new System.Drawing.Size(54, 29);
            fileToolStripMenuItem.Text = "File";
            // 
            // buildToolStripMenuItem
            // 
            buildToolStripMenuItem.ForeColor = System.Drawing.SystemColors.Window;
            buildToolStripMenuItem.Name = "buildToolStripMenuItem";
            buildToolStripMenuItem.Size = new System.Drawing.Size(67, 29);
            buildToolStripMenuItem.Text = "Build";
            // 
            // treeView1
            // 
            treeView1.BackColor = System.Drawing.Color.FromArgb(40, 40, 40);
            treeView1.Dock = DockStyle.Left;
            treeView1.Location = new System.Drawing.Point(0, 33);
            treeView1.Name = "treeView1";
            treeView1.Size = new System.Drawing.Size(320, 721);
            treeView1.TabIndex = 2;
            // 
            // toolStrip1
            // 
            toolStrip1.ImageScalingSize = new System.Drawing.Size(24, 24);
            toolStrip1.Location = new System.Drawing.Point(320, 33);
            toolStrip1.Name = "toolStrip1";
            toolStrip1.Size = new System.Drawing.Size(1258, 25);
            toolStrip1.TabIndex = 1;
            toolStrip1.Text = "toolStrip1";
            // 
            // openFileDialog1
            // 
            openFileDialog1.FileName = "openFileDialog1";
            // 
            // LevelEditorForm
            // 
            AutoScaleDimensions = new System.Drawing.SizeF(10F, 25F);
            AutoScaleMode = AutoScaleMode.Font;
            ClientSize = new System.Drawing.Size(1898, 1024);
            Controls.Add(toolStrip1);
            Controls.Add(RenderBox);
            Controls.Add(panel1);
            Controls.Add(treeView1);
            Controls.Add(tabControl1);
            Controls.Add(LevelEditorMenuStrip);
            MainMenuStrip = LevelEditorMenuStrip;
            Name = "LevelEditorForm";
            Text = "LevelEditorForm1";
            Load += LevelEditorForm_Load;
            tabControl1.ResumeLayout(false);
            tabPage1.ResumeLayout(false);
            tabPage2.ResumeLayout(false);
            tabPage3.ResumeLayout(false);
            tabPage4.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)RenderBox).EndInit();
            LevelEditorMenuStrip.ResumeLayout(false);
            LevelEditorMenuStrip.PerformLayout();
            ResumeLayout(false);
            PerformLayout();
        }

        #endregion

        private System.Windows.Forms.TabControl tabControl1;
        private System.Windows.Forms.TabPage tabPage1;
        private System.Windows.Forms.Panel panel1;
        private System.Windows.Forms.RichTextBox VulkanLoggerBox;
        private System.Windows.Forms.TabPage tabPage2;
        private System.Windows.Forms.PictureBox RenderBox;
        private System.Windows.Forms.MenuStrip LevelEditorMenuStrip;
        private System.Windows.Forms.ToolStripMenuItem fileToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem buildToolStripMenuItem;
        private System.Windows.Forms.TreeView treeView1;
        private System.Windows.Forms.TabPage tabPage3;
        private TabPage tabPage4;
        private ToolStrip toolStrip1;
        private OpenFileDialog openFileDialog1;
        private ImageList imageList1;
        private ListView GameObjectListView;
        private ListView MaterialListView;
        private ListView TextureListView;
    }
}