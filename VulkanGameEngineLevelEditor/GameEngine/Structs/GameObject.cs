using GlmSharp;
using Newtonsoft.Json;
using System.Collections.Generic;
using System.ComponentModel;
using VulkanGameEngineLevelEditor.GameEngine.GameObjectComponents;
using VulkanGameEngineLevelEditor.Models;

namespace VulkanGameEngineLevelEditor.GameEngineAPI
{
    public class GameObject
    {
        
        public string Name { get; set; }
        [ReadOnly(true)]
        [IgnoreProperty]
        public int GameObjectId { get; set; }
        //public List<ComponentTypeEnum> GameObjectComponentTypeList { get; set; } = new List<ComponentTypeEnum>();
        public GameObject()
        {
        }

        public GameObject(string name, int gameObjectId)
        {
            Name = name;
            GameObjectId = gameObjectId;
        }
    }
}