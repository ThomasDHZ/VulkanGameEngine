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
using VulkanGameEngineLevelEditor.LevelEditor.Attributes;

namespace VulkanGameEngineLevelEditor.GameEngine.Systems
{
    public enum GameObjectTypeEnum
    {
        kGameObjectNone,
        kGameObjectMegaMan,
        kGameObjectMegaManShot
    };

    public enum ComponentTypeEnum
    {
        kUndefined,
        kInputComponent,
        kSpriteComponent,
        kTransform2DComponent,
        kTransform3DComponent,
        kCameraFollowComponent,
    }

    public struct GameObject
    {
        public uint GameObjectId = uint.MaxValue;
        public uint ParentGameObjectId = uint.MaxValue;
        public bool GameObjectAlive = true;
        public uint Padding; //entt::entity GameObjectComponents in C++ side

        public GameObject()
        {
        }
    }

    public static unsafe class GameObjectSystem
    {
        public static void CreateGameObject(String gameObjectJson, vec2 gameObjectPosition, uint parentGameObjectId = uint.MaxValue)
        {
            DLLSystem.CallDLLFunc(() => GameObjectSystem_CreateGameObject(gameObjectJson, gameObjectPosition, parentGameObjectId));
        }

        public static void Update(float deltaTime)
        {
            DLLSystem.CallDLLFunc(() => GameObjectSystem_Update(deltaTime));
        }

        public static void UpdateGameObjectComponent(uint gameObjectId, ComponentTypeEnum componentType)
        {
            DLLSystem.CallDLLFunc(() => GameObjectSystem_UpdateGameObjectComponent(gameObjectId, componentType));
        }

        public static void DestroyGameObject(uint gameObjectId)
        {
            DLLSystem.CallDLLFunc(() => GameObjectSystem_DestroyGameObject(gameObjectId));
        }

        public static void DestroyDeadGameObjects()
        {
            DLLSystem.CallDLLFunc(() => GameObjectSystem_DestroyDeadGameObjects());
        }

        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern void GameObjectSystem_CreateGameObject([MarshalAs(UnmanagedType.LPStr)] String gameObjectJson, vec2 gameObjectPosition, uint parentGameObjectId);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern void GameObjectSystem_Update(float deltaTime);
        // [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern GameObject* GameObjectSystem_UpdateGameObject(uint gameObjectIndex);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern void* GameObjectSystem_UpdateGameObjectComponent(uint gameObjectId, ComponentTypeEnum componentType);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern void GameObjectSystem_DestroyGameObject(uint gameObjectId);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern void GameObjectSystem_DestroyDeadGameObjects();
    }
}