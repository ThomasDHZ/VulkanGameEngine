using System;
using System.Windows.Forms;
using VulkanGameEngineLevelEditor.GameEngine;
using VulkanGameEngineLevelEditor.LevelEditor.EditorEnhancements;

public class LevelEditorTreeView : TreeView
{
    public PropertiesPanel PropertiesPanel { get; set; }

    public LevelEditorTreeView()
    {
        this.AfterSelect += OnNodeSelected;
        this.ShowRootLines = true;
        this.ShowPlusMinus = true;
    }

    public void PopulateWithGameObject(uint gameObjectId)
    {
        this.Nodes.Clear();

        if (gameObjectId == uint.MaxValue)
            return;

        var rootNode = new TreeNode($"GameObject [{gameObjectId}]")
        {
            Tag = gameObjectId, 
            ImageIndex = 0,        
            SelectedImageIndex = 0
        };

        this.Nodes.Add(rootNode);
        AddComponentNodes(rootNode, gameObjectId);
        rootNode.Expand();
    }

    private void AddComponentNodes(TreeNode parent, uint gameObjectId)
    {
        var componentTypes = GameObjectSystem.GetGameObjectComponentList(gameObjectId);

        foreach (var compType in componentTypes)
        {
            var node = new TreeNode(compType.ToString())
            {
                Tag = gameObjectId     
            };
            parent.Nodes.Add(node);
        }
    }

    private void OnNodeSelected(object sender, TreeViewEventArgs e)
    {
        if (e.Node?.Tag is uint gameObjectId && gameObjectId != uint.MaxValue)
        {
            PropertiesPanel?.SetSelectedEntity(gameObjectId);
        }
    }

    public void SelectGameObject(uint gameObjectId)
    {
        PopulateWithGameObject(gameObjectId);
        if (Nodes.Count > 0) SelectedNode = Nodes[0];
    }
}