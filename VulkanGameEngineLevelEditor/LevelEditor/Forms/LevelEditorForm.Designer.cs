using System.Drawing;
using System.Windows.Forms;
using VulkanGameEngineLevelEditor.LevelEditor;
using VulkanGameEngineLevelEditor.LevelEditor.EditorEnhancements;

namespace VulkanGameEngineLevelEditor
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
            toolStrip1 = new ToolStrip();
            toolStripButton1 = new ToolStripButton();
            toolStripButton2 = new ToolStripButton();
            richTextBox2 = new RichTextBox();
            dataGridViewTextBoxColumn5 = new DataGridViewTextBoxColumn();
            dataGridViewTextBoxColumn6 = new DataGridViewTextBoxColumn();
            dataGridViewTextBoxColumn7 = new DataGridViewTextBoxColumn();
            dataGridViewTextBoxColumn8 = new DataGridViewTextBoxColumn();
            dataGridViewTextBoxColumn9 = new DataGridViewTextBoxColumn();
            dataGridViewTextBoxColumn3 = new DataGridViewTextBoxColumn();
            dataGridViewTextBoxColumn4 = new DataGridViewTextBoxColumn();
            menuStrip1 = new MenuStrip();
            fileToolStripMenuItem = new ToolStripMenuItem();
            fileToolStripMenuItem1 = new ToolStripMenuItem();
            buildToolStripMenuItem = new ToolStripMenuItem();
            buildShadersToolStripMenuItem = new ToolStripMenuItem();
            buildRenderPassShadersToolStripMenuItem = new ToolStripMenuItem();
            buildRenderPassToolStripMenuItem = new ToolStripMenuItem();
            saveToolStripMenuItem = new ToolStripMenuItem();
            loadToolStripMenuItem = new ToolStripMenuItem();
            dynamicControlPanelView1 = new DynamicControlPanelView(this);
            tableLayoutPanel1 = new TableLayoutPanel();
            levelEditorTreeView1 = new LevelEditorTreeView();
            tableLayoutPanel2 = new TableLayoutPanel();
            RendererBox = new PictureBox();
            leftSplitter = new Splitter();
            rightSplitter = new Splitter();
            bottomSplitter = new Splitter();
            toolTip1 = new ToolTip(components);
            toolStrip1.SuspendLayout();
            menuStrip1.SuspendLayout();
            tableLayoutPanel1.SuspendLayout();
            tableLayoutPanel2.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)RendererBox).BeginInit();
            SuspendLayout();
            // 
            // toolStrip1
            // 
            toolStrip1.ImageScalingSize = new Size(24, 24);
            toolStrip1.Items.AddRange(new ToolStripItem[] { toolStripButton1, toolStripButton2 });
            toolStrip1.Location = new Point(0, 33);
            toolStrip1.Name = "toolStrip1";
            toolStrip1.Size = new Size(2514, 25);
            toolStrip1.TabIndex = 3;
            toolStrip1.Text = "toolStrip1";
            // 
            // toolStripButton1
            // 
            toolStripButton1.Name = "toolStripButton1";
            toolStripButton1.Size = new Size(34, 20);
            // 
            // toolStripButton2
            // 
            toolStripButton2.Name = "toolStripButton2";
            toolStripButton2.Size = new Size(34, 20);
            // 
            // richTextBox2
            // 
            richTextBox2.BackColor = Color.FromArgb(30, 30, 30);
            richTextBox2.Dock = DockStyle.Bottom;
            richTextBox2.ForeColor = Color.White;
            richTextBox2.Location = new Point(0, 1021);
            richTextBox2.Margin = new Padding(4);
            richTextBox2.Name = "richTextBox2";
            richTextBox2.ReadOnly = true;
            richTextBox2.Size = new Size(2514, 180);
            richTextBox2.TabIndex = 9;
            richTextBox2.Text = "";
            // 
            // dataGridViewTextBoxColumn5
            // 
            dataGridViewTextBoxColumn5.MinimumWidth = 8;
            dataGridViewTextBoxColumn5.Name = "dataGridViewTextBoxColumn5";
            dataGridViewTextBoxColumn5.Width = 150;
            // 
            // dataGridViewTextBoxColumn6
            // 
            dataGridViewTextBoxColumn6.MinimumWidth = 8;
            dataGridViewTextBoxColumn6.Name = "dataGridViewTextBoxColumn6";
            dataGridViewTextBoxColumn6.Width = 150;
            // 
            // dataGridViewTextBoxColumn7
            // 
            dataGridViewTextBoxColumn7.MinimumWidth = 8;
            dataGridViewTextBoxColumn7.Name = "dataGridViewTextBoxColumn7";
            dataGridViewTextBoxColumn7.Width = 150;
            // 
            // dataGridViewTextBoxColumn8
            // 
            dataGridViewTextBoxColumn8.MinimumWidth = 8;
            dataGridViewTextBoxColumn8.Name = "dataGridViewTextBoxColumn8";
            dataGridViewTextBoxColumn8.Width = 150;
            // 
            // dataGridViewTextBoxColumn9
            // 
            dataGridViewTextBoxColumn9.MinimumWidth = 8;
            dataGridViewTextBoxColumn9.Name = "dataGridViewTextBoxColumn9";
            dataGridViewTextBoxColumn9.Width = 150;
            // 
            // dataGridViewTextBoxColumn3
            // 
            dataGridViewTextBoxColumn3.MinimumWidth = 8;
            dataGridViewTextBoxColumn3.Name = "dataGridViewTextBoxColumn3";
            dataGridViewTextBoxColumn3.Width = 150;
            // 
            // dataGridViewTextBoxColumn4
            // 
            dataGridViewTextBoxColumn4.MinimumWidth = 8;
            dataGridViewTextBoxColumn4.Name = "dataGridViewTextBoxColumn4";
            dataGridViewTextBoxColumn4.Width = 150;
            // 
            // menuStrip1
            // 
            menuStrip1.ImageScalingSize = new Size(24, 24);
            menuStrip1.Items.AddRange(new ToolStripItem[] { fileToolStripMenuItem, fileToolStripMenuItem1, buildToolStripMenuItem });
            menuStrip1.Location = new Point(0, 0);
            menuStrip1.Name = "menuStrip1";
            menuStrip1.Size = new Size(2514, 33);
            menuStrip1.TabIndex = 12;
            menuStrip1.Text = "menuStrip1";
            // 
            // fileToolStripMenuItem
            // 
            fileToolStripMenuItem.Name = "fileToolStripMenuItem";
            fileToolStripMenuItem.Size = new Size(16, 29);
            // 
            // fileToolStripMenuItem1
            // 
            fileToolStripMenuItem1.Name = "fileToolStripMenuItem1";
            fileToolStripMenuItem1.Size = new Size(54, 29);
            fileToolStripMenuItem1.Text = "File";
            // 
            // buildToolStripMenuItem
            // 
            buildToolStripMenuItem.DropDownItems.AddRange(new ToolStripItem[] { buildShadersToolStripMenuItem, buildRenderPassToolStripMenuItem });
            buildToolStripMenuItem.Name = "buildToolStripMenuItem";
            buildToolStripMenuItem.Size = new Size(67, 29);
            buildToolStripMenuItem.Text = "Build";
            // 
            // buildShadersToolStripMenuItem
            // 
            buildShadersToolStripMenuItem.DropDownItems.AddRange(new ToolStripItem[] { buildRenderPassShadersToolStripMenuItem });
            buildShadersToolStripMenuItem.Name = "buildShadersToolStripMenuItem";
            buildShadersToolStripMenuItem.Size = new Size(270, 34);
            buildShadersToolStripMenuItem.Text = "Build Shaders";
            buildShadersToolStripMenuItem.Click += buildShadersToolStripMenuItem_Click;
            // 
            // buildRenderPassShadersToolStripMenuItem
            // 
            buildRenderPassShadersToolStripMenuItem.Name = "buildRenderPassShadersToolStripMenuItem";
            buildRenderPassShadersToolStripMenuItem.Size = new Size(315, 34);
            buildRenderPassShadersToolStripMenuItem.Text = "Build RenderPass Shaders";
            // 
            // buildRenderPassToolStripMenuItem
            // 
            buildRenderPassToolStripMenuItem.Name = "buildRenderPassToolStripMenuItem";
            buildRenderPassToolStripMenuItem.Size = new Size(270, 34);
            buildRenderPassToolStripMenuItem.Text = "Build RenderPass";
            buildRenderPassToolStripMenuItem.Click += buildRenderPassToolStripMenuItem_Click;
            // 
            // saveToolStripMenuItem
            // 
            saveToolStripMenuItem.Name = "saveToolStripMenuItem";
            saveToolStripMenuItem.Size = new Size(32, 19);
            // 
            // loadToolStripMenuItem
            // 
            loadToolStripMenuItem.Name = "loadToolStripMenuItem";
            loadToolStripMenuItem.Size = new Size(32, 19);
            // 
            // dynamicControlPanelView1
            // 
            dynamicControlPanelView1.AutoScroll = true;
            dynamicControlPanelView1.ColumnCount = 1;
            dynamicControlPanelView1.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 100F));
            dynamicControlPanelView1.Dock = DockStyle.Right;
            dynamicControlPanelView1.Location = new Point(1985, 58);
            dynamicControlPanelView1.MinimumSize = new Size(100, 0);
            dynamicControlPanelView1.Name = "dynamicControlPanelView1";
            dynamicControlPanelView1.RowCount = 1;
            dynamicControlPanelView1.RowStyles.Add(new RowStyle(SizeType.Percent, 100F));
            dynamicControlPanelView1.SelectedObject = null;
            dynamicControlPanelView1.Size = new Size(529, 958);
            dynamicControlPanelView1.TabIndex = 14;
            // 
            // tableLayoutPanel1
            // 
            tableLayoutPanel1.ColumnCount = 1;
            tableLayoutPanel1.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 50F));
            tableLayoutPanel1.Controls.Add(levelEditorTreeView1, 0, 0);
            tableLayoutPanel1.Dock = DockStyle.Left;
            tableLayoutPanel1.Location = new Point(0, 58);
            tableLayoutPanel1.Name = "tableLayoutPanel1";
            tableLayoutPanel1.RowCount = 1;
            tableLayoutPanel1.RowStyles.Add(new RowStyle(SizeType.Percent, 50F));
            tableLayoutPanel1.Size = new Size(300, 958);
            tableLayoutPanel1.TabIndex = 16;
            // 
            // levelEditorTreeView1
            // 
            levelEditorTreeView1.BackColor = Color.FromArgb(40, 40, 40);
            levelEditorTreeView1.Dock = DockStyle.Fill;
            levelEditorTreeView1.Font = new Font("Segoe UI", 12F);
            levelEditorTreeView1.ForeColor = Color.White;
            levelEditorTreeView1.LineColor = Color.White;
            levelEditorTreeView1.Location = new Point(3, 3);
            levelEditorTreeView1.Name = "levelEditorTreeView1";
            levelEditorTreeView1.Size = new Size(294, 952);
            levelEditorTreeView1.TabIndex = 0;
            // 
            // tableLayoutPanel2
            // 
            tableLayoutPanel2.ColumnCount = 1;
            tableLayoutPanel2.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 50F));
            tableLayoutPanel2.Controls.Add(RendererBox, 0, 0);
            tableLayoutPanel2.Dock = DockStyle.Fill;
            tableLayoutPanel2.Location = new Point(305, 58);
            tableLayoutPanel2.Name = "tableLayoutPanel2";
            tableLayoutPanel2.RowCount = 1;
            tableLayoutPanel2.RowStyles.Add(new RowStyle(SizeType.Percent, 50F));
            tableLayoutPanel2.Size = new Size(1675, 958);
            tableLayoutPanel2.TabIndex = 17;
            // 
            // RendererBox
            // 
            RendererBox.Dock = DockStyle.Fill;
            RendererBox.Location = new Point(3, 3);
            RendererBox.Name = "RendererBox";
            RendererBox.Size = new Size(1669, 952);
            RendererBox.TabIndex = 0;
            RendererBox.TabStop = false;
            RendererBox.Resize += RendererBox_Resize;
            // 
            // leftSplitter
            // 
            leftSplitter.BackColor = Color.Gray;
            leftSplitter.Location = new Point(300, 58);
            leftSplitter.MinSize = 100;
            leftSplitter.Name = "leftSplitter";
            leftSplitter.Size = new Size(5, 958);
            leftSplitter.TabIndex = 19;
            leftSplitter.TabStop = false;
            // 
            // rightSplitter
            // 
            rightSplitter.BackColor = Color.Gray;
            rightSplitter.Dock = DockStyle.Right;
            rightSplitter.Location = new Point(1980, 58);
            rightSplitter.MinSize = 100;
            rightSplitter.Name = "rightSplitter";
            rightSplitter.Size = new Size(5, 958);
            rightSplitter.TabIndex = 18;
            rightSplitter.TabStop = false;
            // 
            // bottomSplitter
            // 
            bottomSplitter.BackColor = Color.Gray;
            bottomSplitter.Dock = DockStyle.Bottom;
            bottomSplitter.Location = new Point(0, 1016);
            bottomSplitter.MinSize = 100;
            bottomSplitter.Name = "bottomSplitter";
            bottomSplitter.Size = new Size(2514, 5);
            bottomSplitter.TabIndex = 20;
            bottomSplitter.TabStop = false;
            // 
            // LevelEditorForm
            // 
            AutoScaleDimensions = new SizeF(10F, 25F);
            AutoScaleMode = AutoScaleMode.Font;
            BackColor = Color.FromArgb(40, 40, 40);
            ClientSize = new Size(2514, 1201);
            Controls.Add(tableLayoutPanel2);
            Controls.Add(rightSplitter);
            Controls.Add(dynamicControlPanelView1);
            Controls.Add(leftSplitter);
            Controls.Add(tableLayoutPanel1);
            Controls.Add(bottomSplitter);
            Controls.Add(richTextBox2);
            Controls.Add(toolStrip1);
            Controls.Add(menuStrip1);
            MainMenuStrip = menuStrip1;
            Margin = new Padding(4);
            Name = "LevelEditorForm";
            Text = "Vulkan Level Editor - LevelEditorView";
            Load += LevelEditorForm_Load;
            Resize += LevelEditorForm_Resize;
            toolStrip1.ResumeLayout(false);
            toolStrip1.PerformLayout();
            menuStrip1.ResumeLayout(false);
            menuStrip1.PerformLayout();
            tableLayoutPanel1.ResumeLayout(false);
            tableLayoutPanel2.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)RendererBox).EndInit();
            ResumeLayout(false);
            PerformLayout();
        }

        #endregion
        private System.Windows.Forms.ToolStrip toolStrip1;
        private System.Windows.Forms.ToolStripButton toolStripButton1;
        private System.Windows.Forms.RichTextBox richTextBox2;
        private System.Windows.Forms.DataGridViewTextBoxColumn dataGridViewTextBoxColumn3;
        private System.Windows.Forms.DataGridViewTextBoxColumn dataGridViewTextBoxColumn4;
        private System.Windows.Forms.MenuStrip menuStrip1;
        private System.Windows.Forms.ToolStripMenuItem fileToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem saveToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem loadToolStripMenuItem;
        private System.Windows.Forms.DataGridViewTextBoxColumn dataGridViewTextBoxColumn5;
        private System.Windows.Forms.DataGridViewTextBoxColumn dataGridViewTextBoxColumn6;
        private System.Windows.Forms.DataGridViewTextBoxColumn dataGridViewTextBoxColumn7;
        private System.Windows.Forms.DataGridViewTextBoxColumn dataGridViewTextBoxColumn8;
        private System.Windows.Forms.DataGridViewTextBoxColumn dataGridViewTextBoxColumn9;
        private System.Windows.Forms.TrackBar trackBar1;
        private System.Windows.Forms.ToolStripButton toolStripButton2;
        public DynamicControlPanelView dynamicControlPanelView1;
        private System.Windows.Forms.TableLayoutPanel tableLayoutPanel1;
        private LevelEditorTreeView levelEditorTreeView1;
        private System.Windows.Forms.TableLayoutPanel tableLayoutPanel2;
        private System.Windows.Forms.PictureBox RendererBox;
        private System.Windows.Forms.Splitter rightSplitter;
        private System.Windows.Forms.Splitter leftSplitter;
        private System.Windows.Forms.Splitter bottomSplitter;
        private ToolStripMenuItem fileToolStripMenuItem1;
        private ToolStripMenuItem buildToolStripMenuItem;
        private ToolStripMenuItem buildShadersToolStripMenuItem;
        private ToolStripMenuItem buildRenderPassToolStripMenuItem;
        private ToolStripMenuItem buildRenderPassShadersToolStripMenuItem;
        private ToolTip toolTip1;
    }
}

