using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Reflection;
using System.Windows.Forms;
using VulkanGameEngineLevelEditor.GameEngine;
using VulkanGameEngineLevelEditor.LevelEditor.EditorEnhancements;

public class LevelEditorTreeView : TreeView
{
 //   public DynamicControlPanelView DynamicControlPanel { get; set; }

    public void PopulateWithGameObject(uint gameObjectId)
    {
        this.Nodes.Clear();

        if (gameObjectId == uint.MaxValue)
            return;

        // Create root node for the entity
        var rootNode = new TreeNode($"GameObject [{gameObjectId}]")
        {
            Tag = new GameObject { GameObjectId = gameObjectId }
        };

        this.Nodes.Add(rootNode);

        // Populate all components attached to this entity
        PopulateComponents(rootNode, gameObjectId);

        rootNode.Expand();
    }

    private void PopulateComponents(TreeNode parentNode, uint gameObjectId)
    {
        // This is the key part for pure ECS
        foreach (var entry in ComponentRegistry.Finders)
        {
            var componentTypeEnum = entry.Key;
            var finderFunc = entry.Value;

            try
            {
                var component = finderFunc(gameObjectId);
                if (component == null)
                    continue;

                string nodeText = componentTypeEnum.ToString();

                var compNode = new TreeNode(nodeText)
                {
                    Tag = component
                };

                parentNode.Nodes.Add(compNode);

                // Recursively populate the component's own data
                PopulateNode(compNode, component);
            }
            catch (Exception ex)
            {
                // Some components might not be valid for this entity - that's normal
                System.Diagnostics.Debug.WriteLine($"Component {componentTypeEnum} skipped: {ex.Message}");
            }
        }
    }

    // Keep your existing PopulateNode for drilling into component data
    private void PopulateNode(TreeNode parentNode, object obj)
    {
        if (obj == null) return;

        var visited = new HashSet<object>(); // prevent cycles if needed

        foreach (var prop in obj.GetType().GetProperties(BindingFlags.Public | BindingFlags.Instance))
        {
            if (ShouldSkipProperty(prop)) continue;

            object value = prop.GetValue(obj);
            if (value == null) continue;

            string displayName = GetDisplayName(prop);

            var childNode = new TreeNode(displayName) { Tag = value };
            parentNode.Nodes.Add(childNode);

            // If it's a complex type, expand it further
            if (ShouldExpandFurther(value.GetType()))
            {
                PopulateNode(childNode, value);
            }
        }
    }

    private static bool ShouldSkipProperty(PropertyInfo prop)
    {
        var t = prop.PropertyType;
        return t.IsEnum ||
               t == typeof(Guid) ||
               t == typeof(IntPtr) ||
               typeof(IEnumerable<string>).IsAssignableFrom(t);
    }

    private static bool ShouldExpandFurther(Type type)
    {
        return (type.IsClass && type != typeof(string)) ||
               (type.IsValueType && !type.IsPrimitive && !type.IsEnum);
    }

    private static string GetDisplayName(PropertyInfo prop)
    {
        var attr = prop.GetCustomAttribute<DisplayNameAttribute>();
        return attr?.DisplayName ?? prop.Name;
    }

    private void LevelEditorTreeView_AfterSelect(object sender, TreeViewEventArgs e)
    {
     //   DynamicControlPanel.SetSelectedObject(e.Node.Tag);
    }
}