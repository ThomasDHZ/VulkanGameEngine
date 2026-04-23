using GlmSharp;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using VulkanGameEngineLevelEditor.LevelEditor.Attributes;

namespace VulkanGameEngineLevelEditor.GameEngine.Components
{
    [StructLayout(LayoutKind.Sequential, Pack = 4)]
    public struct Transform3DComponent
    {
        [EditorDisplayName("Position 3D")]
        public vec3 GameObjectPosition = new vec3(0.0f);

        [EditorDisplayName("Rotation 3D")]
        public vec3 GameObjectRotation = new vec3(0.0f);

        [EditorDisplayName("Scale 3D")]
        public vec3 GameObjectScale = new vec3(1.0f);

        [IgnoreProperty]
        public bool Dirty = true;

        public Transform3DComponent()
        {
        }
    }
}
