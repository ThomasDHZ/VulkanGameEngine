using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VulkanEngineCS.Models
{
    public struct GameObjectBehaviors
    {
        public GameObjectMemberType GameObjectType { get; set; } = GameObjectMemberType.GameObjectVarUnknown;
        public string GameObjectDllType { get; set; } = string.Empty;
        public string GameObjectLuaScript { get; set; } = string.Empty;
        public Guid SpriteVramId { get; set; } = Guid.Empty;
        public List<GameObjectVariable> gameObjectVariableList { get; set; } = new List<GameObjectVariable>();
        public GameObjectBehaviors()
        {
        }
    }
}
