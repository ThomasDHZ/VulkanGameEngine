using GlmSharp;
using Newtonsoft.Json;
using Silk.NET.SDL;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Numerics;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using VulkanGameEngineLevelEditor.GameEngine.GameObjectComponents;
using VulkanGameEngineLevelEditor.GameEngine.Structs;
using VulkanGameEngineLevelEditor.GameEngineAPI;

namespace VulkanGameEngineLevelEditor.GameEngine.Systems
{
    public enum GameObjectTypeEnum
    {
        kGameObjectNone,
        kGameObjectMegaMan,
        kGameObjectMegaManShot
    };

    public enum ComponentTypeEnum : ulong
    {
        kUndefined = 0,
        kInputComponent = 1 << 0,
        kSpriteComponent = 1 << 1,
        kTransform2DComponent = 1 << 2,
        kTransform3DComponent = 1 << 3,
    }

    public static class GameObjectSystem
    {
        public static ListPtr<Vertex2D> SpriteVertexList = new ListPtr<Vertex2D>()
        {
            new Vertex2D(new vec2(0.0f, 1.0f), new vec2(0.0f, 0.0f)),
            new Vertex2D(new vec2(1.0f, 1.0f), new vec2(1.0f, 0.0f)),
            new Vertex2D(new vec2(1.0f, 0.0f), new vec2(1.0f, 1.0f)),
            new Vertex2D(new vec2(0.0f, 0.0f), new vec2(0.0f, 1.0f)),
        };

        public static ListPtr<uint> SpriteIndexList = new ListPtr<uint>()
        {
            0, 3, 1,
            1, 3, 2
        };

        public static Dictionary<int, GameObject> GameObjectMap { get; private set; } = new Dictionary<int, GameObject>();
        public static Dictionary<int, Transform2DComponent> Transform2DComponentMap { get; set; } = new Dictionary<int, Transform2DComponent>();
        public static Dictionary<int, InputComponent> InputComponentMap { get; private set; } = new Dictionary<int, InputComponent>();

        public static void CreateGameObject(string name, GameObjectTypeEnum gameObjectType, ComponentTypeEnum componentTypeEnum, Guid vramId, vec2 objectPosition)
        {
            GameObject_CreateGameObject(RenderSystem.renderer, name, uint.MaxValue, gameObjectType, componentTypeEnum, vramId, objectPosition);
        }

        public static void CreateGameObject(string gameObjectPath, vec2 positionOverride)
        {
            GameObject_CreateGameObjectFromJson(RenderSystem.renderer, gameObjectPath, positionOverride);
        }

        public static void LoadTransformComponent(GameObjectComponentLoader loader, int gameObjectId, vec2 gameObjectPosition)
        {
            Transform2DComponent transform2D = new Transform2DComponent();
            transform2D.GameObjectPosition = gameObjectPosition;
            transform2D.GameObjectRotation = loader.GameObjectRotation;
            transform2D.GameObjectScale = loader.GameObjectScale;
            Transform2DComponentMap[gameObjectId] = transform2D;
        }

        public static void LoadInputComponent(GameObjectComponentLoader loader, int gameObjectId)
        {
            InputComponentMap[gameObjectId] = new InputComponent();
        }

        public static void LoadSpriteComponent(GameObjectComponentLoader loader, uint gameObjectId)
        {
            //SpriteSystem.AddSprite(gameObjectId, loader.VramId);
        }

        public static void DestroyGameObject(int gameObjectId)
        {
            //	GameObjectMap.erase(gameObjectId);
            //  Transform2DComponentMap.erase(gameObjectId);
            // InputComponentMap.erase(gameObjectId);
        }

        public static void DestroyGameObjects()
        {
            //for (auto & gameObject : GameObjectMap)
            //{
            //    DestroyGameObject(gameObject.second.GameObjectId);
            //}
        }
        [DllImport(DLLSystem.Game2DDLL, CallingConvention = CallingConvention.StdCall)] public static extern void GameObject_CreateGameObjectFromJson(GraphicsRenderer renderer, [MarshalAs(System.Runtime.InteropServices.UnmanagedType.LPStr)] string jsonString, vec2 positionOverride);
        [DllImport(DLLSystem.Game2DDLL, CallingConvention = CallingConvention.StdCall)] public static extern void GameObject_CreateGameObject(GraphicsRenderer renderer, [MarshalAs(System.Runtime.InteropServices.UnmanagedType.LPStr)] String name, uint parentGameObjectId, GameObjectTypeEnum objectEnum, ComponentTypeEnum gameObjectComponentMask, Guid vramId, vec2 objectPosition);
    }
}
