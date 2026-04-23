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
    public struct Transform2DComponent
    {
        [EditorDisplayName("Position 2D")]
        public vec2 GameObjectPosition = new vec2(0.0f);

        [EditorDisplayName("Rotation 2D")]
        public vec2 GameObjectRotation = new vec2(0.0f);

        [EditorDisplayName("Scale 2D")]
        public vec2 GameObjectScale = new vec2(1.0f);

        [IgnoreProperty]
        public bool Dirty = true;

        public Transform2DComponent()
        {
        }
    }
}
