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

    namespace VulkanGameEngineLevelEditor.GameEngine.Systems
    {
        public static unsafe class GameObjectSystem
        {
            public static void CreateGameObjectFromJson(string gameObjectPath, vec2 gameObjectPosition)
            {
                DLLSystem.CallDLLFunc(() => GameObjectSystem_CreateGameObjectFromJson(gameObjectPath, gameObjectPosition));
            }

            public static void CreateGameObject(string name, uint parentGameObjectId, GameObjectTypeEnum objectEnum, UInt16 gameObjectComponentMask, Guid vramId, vec2 objectPosition)
            {
                DLLSystem.CallDLLFunc(() => GameObjectSystem_CreateGameObject(name, parentGameObjectId, objectEnum, gameObjectComponentMask, vramId, objectPosition));
            }

            public static void Update(float deltaTime)
            {
                DLLSystem.CallDLLFunc(() => GameObjectSystem_Update(deltaTime));
            }

            public static void LoadTransformComponent(string jsonString, uint gameObjectId, vec2 gameObjectPosition)
            {
                DLLSystem.CallDLLFunc(() => GameObjectSystem_LoadTransformComponent(jsonString, gameObjectId, gameObjectPosition));
            }

            public static void LoadInputComponent(string jsonString, uint gameObjectId)
            {
                DLLSystem.CallDLLFunc(() => GameObjectSystem_LoadInputComponent(jsonString, gameObjectId));
            }

            public static void LoadSpriteComponent(string jsonString, GameObject gameObject)
            {
                DLLSystem.CallDLLFunc(() => GameObjectSystem_LoadSpriteComponent(jsonString, gameObject));
            }

            public static void DestroyGameObject(uint gameObjectId)
            {
                DLLSystem.CallDLLFunc(() => GameObjectSystem_DestroyGameObject(gameObjectId));
            }

            public static void DestroyGameObjects()
            {
                DLLSystem.CallDLLFunc(() => GameObjectSystem_DestroyGameObjects());
            }

            public static void DestroyDeadGameObjects()
            {
                DLLSystem.CallDLLFunc(() => GameObjectSystem_DestroyDeadGameObjects());
            }

            public static GameObject FindGameObject(uint gameObjectId)
            {
                return DLLSystem.CallDLLFunc(() => GameObjectSystem_FindGameObject(gameObjectId));
            }

            public static Transform2DComponent FindTransform2DComponent(uint gameObjectId)
            {
                return DLLSystem.CallDLLFunc(() => GameObjectSystem_FindTransform2DComponent(gameObjectId));
            }

            public static InputComponent FindInputComponent(uint gameObjectId)
            {
                return DLLSystem.CallDLLFunc(() => GameObjectSystem_FindInputComponent(gameObjectId));
            }

            //public static List<GameObject> GameObjectList()
            //{
            //    int count = DLLSystem.CallDLLFunc(() => GameObjectSystem_GameObjectList(0));
            //    var list = new List<GameObject>((int)count);
            //    for (size_t i = 0; i < count; i++)
            //    {
            //        list.Add(DLLSystem.CallDLLFunc(() => GameObjectSystem_GameObjectList(i)));
            //    }
            //    return list;
            //}

            //public static List<Transform2DComponent> Transform2DComponentList()
            //{
            //    var count = DLLSystem.CallDLLFunc(() => GameObjectSystem_Transform2DComponentList(0));
            //    var list = new List<Transform2DComponent>((int)count);
            //    for (size_t i = 0; i < count; i++)
            //    {
            //        list.Add(DLLSystem.CallDLLFunc(() => GameObjectSystem_Transform2DComponentList(i)));
            //    }
            //    return list;
            //}

            //public static List<InputComponent> InputComponentList()
            //{
            //    var count = DLLSystem.CallDLLFunc(() => GameObjectSystem_InputComponentList(0));
            //    var list = new List<InputComponent>((int)count);
            //    for (size_t i = 0; i < count; i++)
            //    {
            //        list.Add(DLLSystem.CallDLLFunc(() => GameObjectSystem_InputComponentList(i)));
            //    }
            //    return list;
            //}

            [DllImport(DLLSystem.Game2DDLL, CallingConvention = CallingConvention.StdCall)] private static extern void GameObjectSystem_CreateGameObjectFromJson([MarshalAs(System.Runtime.InteropServices.UnmanagedType.LPStr)] string gameObjectPath, vec2 gameObjectPosition);
            [DllImport(DLLSystem.Game2DDLL, CallingConvention = CallingConvention.StdCall)] private static extern void GameObjectSystem_CreateGameObject([MarshalAs(System.Runtime.InteropServices.UnmanagedType.LPStr)] string name, uint parentGameObjectId, GameObjectTypeEnum objectEnum, UInt16 gameObjectComponentMask, Guid vramId, vec2 objectPosition);
            [DllImport(DLLSystem.Game2DDLL, CallingConvention = CallingConvention.StdCall)] private static extern void GameObjectSystem_Update(float deltaTime);
            [DllImport(DLLSystem.Game2DDLL, CallingConvention = CallingConvention.StdCall)] private static extern void GameObjectSystem_LoadTransformComponent([MarshalAs(System.Runtime.InteropServices.UnmanagedType.LPStr)] string jsonString, uint gameObjectId, vec2 gameObjectPosition);
            [DllImport(DLLSystem.Game2DDLL, CallingConvention = CallingConvention.StdCall)] private static extern void GameObjectSystem_LoadInputComponent([MarshalAs(System.Runtime.InteropServices.UnmanagedType.LPStr)] string jsonString, uint gameObjectId);
            [DllImport(DLLSystem.Game2DDLL, CallingConvention = CallingConvention.StdCall)] private static extern void GameObjectSystem_LoadSpriteComponent([MarshalAs(System.Runtime.InteropServices.UnmanagedType.LPStr)] string jsonString, GameObject gameObject);
            [DllImport(DLLSystem.Game2DDLL, CallingConvention = CallingConvention.StdCall)] private static extern void GameObjectSystem_DestroyGameObject(uint gameObjectId);
            [DllImport(DLLSystem.Game2DDLL, CallingConvention = CallingConvention.StdCall)] private static extern void GameObjectSystem_DestroyGameObjects();
            [DllImport(DLLSystem.Game2DDLL, CallingConvention = CallingConvention.StdCall)] private static extern void GameObjectSystem_DestroyDeadGameObjects();
            [DllImport(DLLSystem.Game2DDLL, CallingConvention = CallingConvention.StdCall)] private static extern GameObject GameObjectSystem_FindGameObject(uint gameObjectId);
            [DllImport(DLLSystem.Game2DDLL, CallingConvention = CallingConvention.StdCall)] private static extern Transform2DComponent GameObjectSystem_FindTransform2DComponent(uint gameObjectId);
            [DllImport(DLLSystem.Game2DDLL, CallingConvention = CallingConvention.StdCall)] private static extern InputComponent GameObjectSystem_FindInputComponent(uint gameObjectId);
            [DllImport(DLLSystem.Game2DDLL, CallingConvention = CallingConvention.StdCall)] private static extern GameObject GameObjectSystem_GameObjectList(size_t returnListCount);
            [DllImport(DLLSystem.Game2DDLL, CallingConvention = CallingConvention.StdCall)] private static extern Transform2DComponent GameObjectSystem_Transform2DComponentList(size_t returnListCount);
            [DllImport(DLLSystem.Game2DDLL, CallingConvention = CallingConvention.StdCall)] private static extern InputComponent GameObjectSystem_InputComponentList(size_t returnListCount);
        }
    }
}
