using System;
using System.Drawing;
using System.Windows.Forms;

namespace VulkanGameEngineLevelEditor.LevelEditor.EditorEnhancements
{
    public class CustomResizableDockedPanelsForm : Form
    {
        public CustomResizableDockedPanelsForm()
        {
            // Initialize the form
            this.Text = "Vulkan Level Editor";
            this.Size = new Size(1000, 600);

            // Left Panel (Tree View)
            Panel leftPanel = new Panel
            {
                Dock = DockStyle.Left,
                Width = 200,
                BackColor = Color.LightGray,
                BorderStyle = BorderStyle.FixedSingle
            };
            TreeView treeView = new TreeView
            {
                Dock = DockStyle.Fill
            };
            treeView.Nodes.Add("RenderPassLoaderModel");
            treeView.Nodes.Add("RenderPipelineModelList");
            treeView.Nodes.Add("RenderArea");
            leftPanel.Controls.Add(treeView);

            // Left Splitter
            Splitter leftSplitter = new Splitter
            {
                Dock = DockStyle.Left,
                Width = 5,
                BackColor = Color.Gray,
                MinSize = 100 // Minimum width for left panel
            };

            // Main Content Panel (Green Area)
            Panel mainPanel = new Panel
            {
                Dock = DockStyle.Fill,
                BackColor = Color.Green
            };
            // Add your graphical content here (e.g., character row)

            // Right Panel (Properties)
            Panel rightPanel = new Panel
            {
                Dock = DockStyle.Right,
                Width = 250,
                BackColor = Color.LightGray,
                BorderStyle = BorderStyle.FixedSingle
            };
            Label propLabel = new Label { Text = "Name", Location = new Point(10, 10) };
            TextBox propText = new TextBox { Text = "RenderPass", Location = new Point(60, 10), Width = 150 };
            rightPanel.Controls.Add(propLabel);
            rightPanel.Controls.Add(propText);

            // Right Splitter
            Splitter rightSplitter = new Splitter
            {
                Dock = DockStyle.Right,
                Width = 5,
                BackColor = Color.Gray,
                MinSize = 100 // Minimum width for right panel
            };

            // Bottom Panel (Graphical Area)
            Panel bottomPanel = new Panel
            {
                Dock = DockStyle.Bottom,
                Height = 150,
                BackColor = Color.DarkGray,
                BorderStyle = BorderStyle.FixedSingle
            };
            // Add your detailed graphics or character row here

            // Bottom Splitter
            Splitter bottomSplitter = new Splitter
            {
                Dock = DockStyle.Bottom,
                Height = 5,
                BackColor = Color.Gray,
                MinSize = 100 // Minimum height for top content
            };

            // Add controls to the form in the correct order
            this.Controls.Add(bottomPanel);
            this.Controls.Add(bottomSplitter);
            this.Controls.Add(rightPanel);
            this.Controls.Add(rightSplitter);
            this.Controls.Add(mainPanel);
            this.Controls.Add(leftSplitter);
            this.Controls.Add(leftPanel);
        }

    }
}
