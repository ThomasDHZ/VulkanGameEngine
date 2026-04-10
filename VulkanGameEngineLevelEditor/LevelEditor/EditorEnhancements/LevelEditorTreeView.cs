using System;
using System.Windows.Forms;
using VulkanGameEngineLevelEditor.GameEngine;
using VulkanGameEngineLevelEditor.LevelEditor.EditorEnhancements;

namespace VulkanGameEngineLevelEditor.LevelEditor
{
    public class LevelEditorTreeView : TreeView
    {
        public PropertiesPanel PropertiesPanel { get; set; }

        public LevelEditorTreeView()
        {
            this.AfterSelect += LevelEditorTreeView_AfterSelect;
        }

        public void PopulateWithGameObjects()
        {
            if (this.InvokeRequired)
            {
                this.BeginInvoke(new Action(PopulateWithGameObjects));
                return;
            }
            this.Nodes.Clear();

            var gameObjectList = GameObjectSystem.GetGameObjectList();
            foreach (var go in gameObjectList)
            {
                if (go.GameObjectId == uint.MaxValue) continue;
                AddGameObject(go.GameObjectId);
            }
            if (Nodes.Count > 0)  SelectedNode = Nodes[0];
        }

        public void AddGameObject(uint gameObjectId, string customName = null)
        {
            if (gameObjectId == uint.MaxValue) return;

            if (this.InvokeRequired)
            {
                this.BeginInvoke(new Action<uint, string>(AddGameObject), gameObjectId, customName);
                return;
            }

            string nodeText = customName ?? $"GameObject [{gameObjectId}]";
            var node = new TreeNode(nodeText)
            {
                Tag = gameObjectId
            };
            this.Nodes.Add(node);
        }

        private void LevelEditorTreeView_AfterSelect(object sender, TreeViewEventArgs e)
        {
            if (e.Node?.Tag is uint gameObjectId && gameObjectId != uint.MaxValue)
            {
                PropertiesPanel?.SetSelectedEntity(gameObjectId);
            }
        }

        public void SelectGameObject(uint gameObjectId)
        {
            if (gameObjectId == uint.MaxValue) return;

            if (this.InvokeRequired)
            {
                this.BeginInvoke(new Action<uint>(SelectGameObject), gameObjectId);
                return;
            }

            var node = FindNodeById(gameObjectId);
            if (node != null)
            {
                this.SelectedNode = node;
                return;
            }
            AddGameObject(gameObjectId);
        }

        private TreeNode FindNodeById(uint gameObjectId)
        {
            foreach (TreeNode node in this.Nodes)
            {
                if (node.Tag is uint id && id == gameObjectId)
                    return node;

                foreach (TreeNode child in node.Nodes)
                {
                    if (child.Tag is uint cid && cid == gameObjectId)
                        return child;
                }
            }
            return null;
        }

        public void Clear()
        {
            if (this.InvokeRequired)
            {
                this.BeginInvoke(new Action(Clear));
                return;
            }
            this.Nodes.Clear();
        }
    }
}