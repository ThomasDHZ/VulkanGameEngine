using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using VulkanGameEngineLevelEditor.GameEngine.Systems;

namespace VulkanGameEngineLevelEditor.LevelEditor.Attributes
{
    [AttributeUsage(AttributeTargets.Property)]
    public class GameObjectComponentAttribute : Attribute
    {
        public ComponentTypeEnum ComponentType;
        public GameObjectComponentAttribute(ComponentTypeEnum componentType)
        {
            ComponentType = componentType;
        }
    }
}
