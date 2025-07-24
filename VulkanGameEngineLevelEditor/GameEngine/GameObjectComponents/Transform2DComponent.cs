using GlmSharp;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Security.Cryptography.X509Certificates;
using System.Text;
using System.Threading.Tasks;
using VulkanGameEngineLevelEditor.LevelEditor.Attributes;

namespace VulkanGameEngineLevelEditor.GameEngine.GameObjectComponents
{
    public struct Transform2DComponent
    {
        private vec2 gameObjectPosition = new vec2(0.0f);
        private vec2 gameObjectRotation = new vec2(0.0f);
        private vec2 gameObjectScale = new vec2(1.0f);

        [Category("Transform")]
        [Tooltip("The 2D position of the game object.")]
        public vec2 GameObjectPosition
        {
            get => gameObjectPosition;
            set
            {
                gameObjectPosition = value;
            }
        }

        [Category("Transform")]
        [Tooltip("The 2D rotation of the game object in degrees.")]
        [Range(-360f, 360f)]
        public vec2 GameObjectRotation
        {
            get => gameObjectRotation;
            set
            {
                gameObjectRotation = value;
            }
        }

        [Category("Transform")]
        [Tooltip("The 2D scale of the game object.")]
        [Range(0.1f, 10f)]
        public vec2 GameObjectScale
        {
            get => gameObjectScale;
            set
            {
                gameObjectScale = value;
            }
        }

        public Transform2DComponent()
        {
        }

        public Transform2DComponent(vec2 gameObjectPosition)
        {
            GameObjectPosition = gameObjectPosition;
        }

        public Transform2DComponent(vec2 gameObjectPosition, vec2 gameObjectRotation)
        {
            GameObjectPosition = gameObjectPosition;
            GameObjectRotation = gameObjectRotation;
        }

        public Transform2DComponent(vec2 gameObjectPosition, vec2 gameObjectRotation, vec2 gameObjectScale)
        {
            GameObjectPosition = gameObjectPosition;
            GameObjectRotation = gameObjectRotation;
            GameObjectScale = gameObjectScale;
        }
    }
}
