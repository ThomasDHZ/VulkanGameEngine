//using System.Drawing;
//using System.Windows.Forms;
//using VulkanGameEngineLevelEditor.LevelEditor;
//using VulkanGameEngineLevelEditor.LevelEditor.EditorEnhancements;

//namespace VulkanGameEngineLevelEditor
//{
//    partial class LevelEditorForm
//    {
//        /// <summary>
//        /// Required designer variable.
//        /// </summary>
//        private System.ComponentModel.IContainer components = null;

//        /// <summary>
//        /// Clean up any resources being used.
//        /// </summary>
//        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
//        protected override void Dispose(bool disposing)
//        {
//            if (disposing && (components != null))
//            {
//                components.Dispose();
//            }
//            base.Dispose(disposing);
//        }

//        #region Windows Form Designer generated code

//        /// <summary>
//        /// Required method for Designer support - do not modify
//        /// the contents of this method with the code editor.
//        /// </summary>
//        private void InitializeComponent()
//        {
//            components = new System.ComponentModel.Container();
//            dataGridViewTextBoxColumn5 = new DataGridViewTextBoxColumn();
//            dataGridViewTextBoxColumn6 = new DataGridViewTextBoxColumn();
//            dataGridViewTextBoxColumn7 = new DataGridViewTextBoxColumn();
//            dataGridViewTextBoxColumn8 = new DataGridViewTextBoxColumn();
//            dataGridViewTextBoxColumn9 = new DataGridViewTextBoxColumn();
//            dataGridViewTextBoxColumn3 = new DataGridViewTextBoxColumn();
//            dataGridViewTextBoxColumn4 = new DataGridViewTextBoxColumn();
//            saveToolStripMenuItem = new ToolStripMenuItem();
//            loadToolStripMenuItem = new ToolStripMenuItem();
//            rightSplitter = new Splitter();
//            tableLayoutPanel2 = new TableLayoutPanel();
//            leftSplitter = new Splitter();
//            fileToolStripMenuItem = new ToolStripMenuItem();
//            fileToolStripMenuItem1 = new ToolStripMenuItem();
//            buildToolStripMenuItem = new ToolStripMenuItem();
//            buildShadersToolStripMenuItem = new ToolStripMenuItem();
//            buildRenderPassShadersToolStripMenuItem = new ToolStripMenuItem();
//            buildRenderPassToolStripMenuItem = new ToolStripMenuItem();
//            editToolStripMenuItem = new ToolStripMenuItem();
//            addGameObjectToolStripMenuItem = new ToolStripMenuItem();
//            menuStrip1 = new MenuStrip();
//            toolTip1 = new ToolTip(components);
//            toolStripButton1 = new ToolStripButton();
//            toolStripButton2 = new ToolStripButton();
//            toolStrip1 = new ToolStrip();
//            bottomSplitter = new Splitter();
//            tabControl1 = new TabControl();
//            tabPage1 = new TabPage();
//            tabPage2 = new TabPage();
//            panel1 = new Panel();
//            RendererBox = new PictureBox();
//            treeView1 = new TreeView();
//            tableLayoutPanel2.SuspendLayout();
//            menuStrip1.SuspendLayout();
//            toolStrip1.SuspendLayout();
//            tabControl1.SuspendLayout();
//            ((System.ComponentModel.ISupportInitialize)RendererBox).BeginInit();
//            SuspendLayout();
//            // 
//            // dataGridViewTextBoxColumn5
//            // 
//            dataGridViewTextBoxColumn5.MinimumWidth = 8;
//            dataGridViewTextBoxColumn5.Name = "dataGridViewTextBoxColumn5";
//            dataGridViewTextBoxColumn5.Width = 150;
//            // 
//            // dataGridViewTextBoxColumn6
//            // 
//            dataGridViewTextBoxColumn6.MinimumWidth = 8;
//            dataGridViewTextBoxColumn6.Name = "dataGridViewTextBoxColumn6";
//            dataGridViewTextBoxColumn6.Width = 150;
//            // 
//            // dataGridViewTextBoxColumn7
//            // 
//            dataGridViewTextBoxColumn7.MinimumWidth = 8;
//            dataGridViewTextBoxColumn7.Name = "dataGridViewTextBoxColumn7";
//            dataGridViewTextBoxColumn7.Width = 150;
//            // 
//            // dataGridViewTextBoxColumn8
//            // 
//            dataGridViewTextBoxColumn8.MinimumWidth = 8;
//            dataGridViewTextBoxColumn8.Name = "dataGridViewTextBoxColumn8";
//            dataGridViewTextBoxColumn8.Width = 150;
//            // 
//            // dataGridViewTextBoxColumn9
//            // 
//            dataGridViewTextBoxColumn9.MinimumWidth = 8;
//            dataGridViewTextBoxColumn9.Name = "dataGridViewTextBoxColumn9";
//            dataGridViewTextBoxColumn9.Width = 150;
//            // 
//            // dataGridViewTextBoxColumn3
//            // 
//            dataGridViewTextBoxColumn3.MinimumWidth = 8;
//            dataGridViewTextBoxColumn3.Name = "dataGridViewTextBoxColumn3";
//            dataGridViewTextBoxColumn3.Width = 150;
//            // 
//            // dataGridViewTextBoxColumn4
//            // 
//            dataGridViewTextBoxColumn4.MinimumWidth = 8;
//            dataGridViewTextBoxColumn4.Name = "dataGridViewTextBoxColumn4";
//            dataGridViewTextBoxColumn4.Width = 150;
//            // 
//            // saveToolStripMenuItem
//            // 
//            saveToolStripMenuItem.Name = "saveToolStripMenuItem";
//            saveToolStripMenuItem.Size = new Size(32, 19);
//            // 
//            // loadToolStripMenuItem
//            // 
//            loadToolStripMenuItem.Name = "loadToolStripMenuItem";
//            loadToolStripMenuItem.Size = new Size(32, 19);
//            // 
//            // rightSplitter
//            // 
//            rightSplitter.BackColor = Color.Gray;
//            rightSplitter.Dock = DockStyle.Right;
//            rightSplitter.Location = new Point(1893, 58);
//            rightSplitter.MinSize = 100;
//            rightSplitter.Name = "rightSplitter";
//            rightSplitter.Size = new Size(5, 961);
//            rightSplitter.TabIndex = 18;
//            rightSplitter.TabStop = false;
//            // 
//            // tableLayoutPanel2
//            // 
//            tableLayoutPanel2.ColumnCount = 1;
//            tableLayoutPanel2.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 50F));
//            tableLayoutPanel2.Controls.Add(RendererBox, 0, 0);
//            tableLayoutPanel2.Dock = DockStyle.Fill;
//            tableLayoutPanel2.Location = new Point(5, 58);
//            tableLayoutPanel2.Name = "tableLayoutPanel2";
//            tableLayoutPanel2.RowCount = 1;
//            tableLayoutPanel2.RowStyles.Add(new RowStyle(SizeType.Percent, 50F));
//            tableLayoutPanel2.Size = new Size(1888, 961);
//            tableLayoutPanel2.TabIndex = 17;
//            // 
//            // leftSplitter
//            // 
//            leftSplitter.BackColor = Color.Gray;
//            leftSplitter.Location = new Point(0, 58);
//            leftSplitter.MinSize = 100;
//            leftSplitter.Name = "leftSplitter";
//            leftSplitter.Size = new Size(5, 961);
//            leftSplitter.TabIndex = 19;
//            leftSplitter.TabStop = false;
//            // 
//            // fileToolStripMenuItem
//            // 
//            fileToolStripMenuItem.Name = "fileToolStripMenuItem";
//            fileToolStripMenuItem.Size = new Size(16, 29);
//            // 
//            // fileToolStripMenuItem1
//            // 
//            fileToolStripMenuItem1.Name = "fileToolStripMenuItem1";
//            fileToolStripMenuItem1.Size = new Size(54, 29);
//            fileToolStripMenuItem1.Text = "File";
//            // 
//            // buildToolStripMenuItem
//            // 
//            buildToolStripMenuItem.DropDownItems.AddRange(new ToolStripItem[] { buildShadersToolStripMenuItem, buildRenderPassToolStripMenuItem });
//            buildToolStripMenuItem.Name = "buildToolStripMenuItem";
//            buildToolStripMenuItem.Size = new Size(67, 29);
//            buildToolStripMenuItem.Text = "Build";
//            // 
//            // buildShadersToolStripMenuItem
//            // 
//            buildShadersToolStripMenuItem.DropDownItems.AddRange(new ToolStripItem[] { buildRenderPassShadersToolStripMenuItem });
//            buildShadersToolStripMenuItem.Name = "buildShadersToolStripMenuItem";
//            buildShadersToolStripMenuItem.Size = new Size(247, 34);
//            buildShadersToolStripMenuItem.Text = "Build Shaders";
//            buildShadersToolStripMenuItem.Click += buildShadersToolStripMenuItem_Click;
//            // 
//            // buildRenderPassShadersToolStripMenuItem
//            // 
//            buildRenderPassShadersToolStripMenuItem.Name = "buildRenderPassShadersToolStripMenuItem";
//            buildRenderPassShadersToolStripMenuItem.Size = new Size(315, 34);
//            buildRenderPassShadersToolStripMenuItem.Text = "Build RenderPass Shaders";
//            // 
//            // buildRenderPassToolStripMenuItem
//            // 
//            buildRenderPassToolStripMenuItem.Name = "buildRenderPassToolStripMenuItem";
//            buildRenderPassToolStripMenuItem.Size = new Size(247, 34);
//            buildRenderPassToolStripMenuItem.Text = "Build RenderPass";
//            buildRenderPassToolStripMenuItem.Click += buildRenderPassToolStripMenuItem_Click;
//            // 
//            // editToolStripMenuItem
//            // 
//            editToolStripMenuItem.DropDownItems.AddRange(new ToolStripItem[] { addGameObjectToolStripMenuItem });
//            editToolStripMenuItem.Name = "editToolStripMenuItem";
//            editToolStripMenuItem.Size = new Size(58, 29);
//            editToolStripMenuItem.Text = "Edit";
//            // 
//            // addGameObjectToolStripMenuItem
//            // 
//            addGameObjectToolStripMenuItem.Name = "addGameObjectToolStripMenuItem";
//            addGameObjectToolStripMenuItem.Size = new Size(251, 34);
//            addGameObjectToolStripMenuItem.Text = "Add GameObject";
//            addGameObjectToolStripMenuItem.Click += addGameObjectToolStripMenuItem_Click;
//            // 
//            // menuStrip1
//            // 
//            menuStrip1.ImageScalingSize = new Size(24, 24);
//            menuStrip1.Items.AddRange(new ToolStripItem[] { fileToolStripMenuItem, fileToolStripMenuItem1, buildToolStripMenuItem, editToolStripMenuItem });
//            menuStrip1.Location = new Point(0, 0);
//            menuStrip1.Name = "menuStrip1";
//            menuStrip1.Size = new Size(1898, 33);
//            menuStrip1.TabIndex = 12;
//            menuStrip1.Text = "menuStrip1";
//            // 
//            // toolStripButton1
//            // 
//            toolStripButton1.Name = "toolStripButton1";
//            toolStripButton1.Size = new Size(34, 20);
//            // 
//            // toolStripButton2
//            // 
//            toolStripButton2.Name = "toolStripButton2";
//            toolStripButton2.Size = new Size(34, 20);
//            // 
//            // toolStrip1
//            // 
//            toolStrip1.ImageScalingSize = new Size(24, 24);
//            toolStrip1.Items.AddRange(new ToolStripItem[] { toolStripButton1, toolStripButton2 });
//            toolStrip1.Location = new Point(0, 33);
//            toolStrip1.Name = "toolStrip1";
//            toolStrip1.Size = new Size(1898, 25);
//            toolStrip1.TabIndex = 3;
//            toolStrip1.Text = "toolStrip1";
//            // 
//            // bottomSplitter
//            // 
//            bottomSplitter.BackColor = Color.Gray;
//            bottomSplitter.Dock = DockStyle.Bottom;
//            bottomSplitter.Location = new Point(0, 1019);
//            bottomSplitter.MinSize = 100;
//            bottomSplitter.Name = "bottomSplitter";
//            bottomSplitter.Size = new Size(1898, 5);
//            bottomSplitter.TabIndex = 20;
//            bottomSplitter.TabStop = false;
//            // 
//            // tabControl1
//            // 
//            tabControl1.Controls.Add(tabPage1);
//            tabControl1.Controls.Add(tabPage2);
//            tabControl1.Dock = DockStyle.Bottom;
//            tabControl1.Location = new Point(5, 820);
//            tabControl1.Name = "tabControl1";
//            tabControl1.SelectedIndex = 0;
//            tabControl1.Size = new Size(1888, 199);
//            tabControl1.TabIndex = 21;
//            // 
//            // tabPage1
//            // 
//            tabPage1.BackColor = Color.FromArgb(40, 40, 40);
//            tabPage1.BorderStyle = BorderStyle.Fixed3D;
//            tabPage1.Location = new Point(4, 34);
//            tabPage1.Name = "tabPage1";
//            tabPage1.Padding = new Padding(3);
//            tabPage1.Size = new Size(1880, 161);
//            tabPage1.TabIndex = 0;
//            tabPage1.Text = "Logger";
//            // 
//            // tabPage2
//            // 
//            tabPage2.Location = new Point(4, 34);
//            tabPage2.Name = "tabPage2";
//            tabPage2.Padding = new Padding(3);
//            tabPage2.Size = new Size(1880, 161);
//            tabPage2.TabIndex = 1;
//            tabPage2.Text = "GameObject";
//            tabPage2.UseVisualStyleBackColor = true;
//            // 
//            // panel1
//            // 
//            panel1.BorderStyle = BorderStyle.Fixed3D;
//            panel1.Dock = DockStyle.Right;
//            panel1.Location = new Point(1593, 58);
//            panel1.Name = "panel1";
//            panel1.Size = new Size(300, 762);
//            panel1.TabIndex = 23;
//            // 
//            // RendererBox
//            // 
//            RendererBox.BackColor = Color.FromArgb(40, 40, 48);
//            RendererBox.BorderStyle = BorderStyle.Fixed3D;
//            RendererBox.Dock = DockStyle.Fill;
//            RendererBox.Location = new Point(3, 3);
//            RendererBox.Name = "RendererBox";
//            RendererBox.Size = new Size(1882, 955);
//            RendererBox.TabIndex = 0;
//            RendererBox.TabStop = false;
//            RendererBox.Click += RendererBox_Click;
//            RendererBox.MouseDown += RendererBox_MouseDown;
//            RendererBox.MouseMove += RendererBox_MouseMove;
//            RendererBox.MouseUp += RendererBox_MouseUp;
//            RendererBox.MouseWheel += RendererBox_MouseWheel;
//            RendererBox.Resize += RendererBox_Resize;
//            // 
//            // treeView1
//            // 
//            treeView1.BackColor = Color.FromArgb(40, 40, 40);
//            treeView1.Dock = DockStyle.Left;
//            treeView1.Location = new Point(5, 58);
//            treeView1.Name = "treeView1";
//            treeView1.Size = new Size(303, 762);
//            treeView1.TabIndex = 22;
//            // 
//            // LevelEditorForm
//            // 
//            AutoScaleDimensions = new SizeF(10F, 25F);
//            AutoScaleMode = AutoScaleMode.Font;
//            BackColor = Color.FromArgb(40, 40, 40);
//            ClientSize = new Size(1898, 1024);
//            Controls.Add(panel1);
//            Controls.Add(treeView1);
//            Controls.Add(tabControl1);
//            Controls.Add(tableLayoutPanel2);
//            Controls.Add(rightSplitter);
//            Controls.Add(leftSplitter);
//            Controls.Add(bottomSplitter);
//            Controls.Add(toolStrip1);
//            Controls.Add(menuStrip1);
//            MainMenuStrip = menuStrip1;
//            Margin = new Padding(4);
//            Name = "LevelEditorForm";
//            Text = "Vulkan Level Editor - LevelEditorView";
//            Load += LevelEditorForm_Load;
//            Resize += LevelEditorForm_Resize;
//            tableLayoutPanel2.ResumeLayout(false);
//            menuStrip1.ResumeLayout(false);
//            menuStrip1.PerformLayout();
//            toolStrip1.ResumeLayout(false);
//            toolStrip1.PerformLayout();
//            tabControl1.ResumeLayout(false);
//            ((System.ComponentModel.ISupportInitialize)RendererBox).EndInit();
//            ResumeLayout(false);
//            PerformLayout();
//        }

//        #endregion
//        private System.Windows.Forms.DataGridViewTextBoxColumn dataGridViewTextBoxColumn3;
//        private System.Windows.Forms.DataGridViewTextBoxColumn dataGridViewTextBoxColumn4;
//        private System.Windows.Forms.ToolStripMenuItem saveToolStripMenuItem;
//        private System.Windows.Forms.ToolStripMenuItem loadToolStripMenuItem;
//        private System.Windows.Forms.DataGridViewTextBoxColumn dataGridViewTextBoxColumn5;
//        private System.Windows.Forms.DataGridViewTextBoxColumn dataGridViewTextBoxColumn6;
//        private System.Windows.Forms.DataGridViewTextBoxColumn dataGridViewTextBoxColumn7;
//        private System.Windows.Forms.DataGridViewTextBoxColumn dataGridViewTextBoxColumn8;
//        private System.Windows.Forms.DataGridViewTextBoxColumn dataGridViewTextBoxColumn9;
//        private System.Windows.Forms.TrackBar trackBar1;
//        private Splitter rightSplitter;
//        private TableLayoutPanel tableLayoutPanel2;
//        private PictureBox RendererBox;
//        private Splitter leftSplitter;
//        private ToolStripMenuItem fileToolStripMenuItem;
//        private ToolStripMenuItem fileToolStripMenuItem1;
//        private ToolStripMenuItem buildToolStripMenuItem;
//        private ToolStripMenuItem buildShadersToolStripMenuItem;
//        private ToolStripMenuItem buildRenderPassShadersToolStripMenuItem;
//        private ToolStripMenuItem buildRenderPassToolStripMenuItem;
//        private ToolStripMenuItem editToolStripMenuItem;
//        private ToolStripMenuItem addGameObjectToolStripMenuItem;
//        private MenuStrip menuStrip1;
//        private ToolTip toolTip1;
//        private ToolStripButton toolStripButton1;
//        private ToolStripButton toolStripButton2;
//        private ToolStrip toolStrip1;
//        private Splitter bottomSplitter;
//        private TabControl tabControl1;
//        private TabPage tabPage1;
//        private TabPage tabPage2;
//        private Panel panel1;
//        private TreeView treeView1;
//    }
//}

