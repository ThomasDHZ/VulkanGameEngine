using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using VulkanGameEngineLevelEditor.LevelEditor.Attributes;
using VulkanGameEngineLevelEditor.LevelEditor.Registries;

namespace VulkanGameEngineLevelEditor.GameEngine.Components
{
    [StructLayout(LayoutKind.Sequential, Pack = 4)]
    public unsafe struct DirectionalLightComponent
    {
        [IgnoreProperty]
        public uint GameObjectId;

        [LinkObject(typeof(DirectionalLightHandle))]
        public uint DirectionalLightId;

        public DirectionalLightComponent() 
        { 
            DirectionalLightId = uint.MaxValue; 
        }
    }
}
