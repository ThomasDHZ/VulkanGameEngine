using System;
using System.Collections.Generic;
using VulkanGameEngineLevelEditor.GameEngine.Systems;
using VulkanGameEngineLevelEditor.GameEngine.Systems.VulkanGameEngineLevelEditor.GameEngine.Systems;

public static class ComponentRegistry
{
    public static readonly Dictionary<ComponentTypeEnum, Func<uint, object>> Finders = new()
    {
        { ComponentTypeEnum.kTransform2DComponent, id =>
        {
            return GameObjectSystem.FindTransform2DComponent(id);
        }},
        { ComponentTypeEnum.kInputComponent, id =>
        {
            return GameObjectSystem.FindInputComponent(id);
        }},
        { ComponentTypeEnum.kSpriteComponent, id =>
        {
            return SpriteSystem.FindSprite(id);
        }}
    };
}