using GlmSharp;
using Newtonsoft.Json;
using Silk.NET.SDL;
using Silk.NET.Vulkan;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Numerics;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using VulkanGameEngineLevelEditor.GameEngine.Structs;
using VulkanGameEngineLevelEditor.GameEngineAPI;
using VulkanGameEngineLevelEditor.LevelEditor.Attributes;

namespace VulkanGameEngineLevelEditor.GameEngine
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
    public struct Transform2DComponent
    {
        public vec2 GameObjectPosition { get; set; } = new vec2(0.0f);
        public vec2 GameObjectRotation { get; set; } = new vec2(0.0f);
        public vec2 GameObjectScale { get; set; } = new vec2(1.0f);
        public bool Dirty { get; set; } = true;

        public Transform2DComponent()
        {
        }
    };

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
        public static UInt32 CreateGameObject(vec2 gameObjectPosition, uint parentGameObjectId = uint.MaxValue)
        {
           return DLLSystem.CallDLLFunc(() => GameObjectSystem_CreateGameObjectBase(gameObjectPosition, parentGameObjectId));
        }

        public static UInt32 CreateGameObject(String gameObjectJson, vec2 gameObjectPosition, uint parentGameObjectId = uint.MaxValue)
        {
            return DLLSystem.CallDLLFunc(() => GameObjectSystem_CreateGameObject(gameObjectJson, gameObjectPosition, parentGameObjectId));
        }

        public static unsafe void CreateGameObjectComponent<T>(uint gameObjectId, ComponentTypeEnum componentType, ref T componentData) where T : unmanaged
        {
            fixed (T* pComponent = &componentData)
            {
                GameObjectSystem_CreateGameObjectComponent(gameObjectId, componentType, pComponent);
            }
        }

        public static void Update(float deltaTime)
        {
            DLLSystem.CallDLLFunc(() => GameObjectSystem_Update(deltaTime));
        }

        public static ref T UpdateGameObjectComponent<T>(uint gameObjectId, ComponentTypeEnum componentType)
        {
            IntPtr ptr = GameObjectSystem_UpdateGameObjectComponent(gameObjectId, componentType);
            if (ptr == IntPtr.Zero)
            {
                System.Diagnostics.Debug.WriteLine($"Warning: Component {componentType} not found for GO {gameObjectId}");
                return ref Unsafe.NullRef<T>();
            }
            return ref Unsafe.AsRef<T>(ptr.ToPointer());
        }

        public static ListPtr<GameObject> GetGameObjectList()
        {
            try
            {
                GameObject* gameObjectList = GameObjectSystem_GetGameObjectList(out size_t gameObjectCount);
                return new ListPtr<GameObject>(gameObjectList, gameObjectCount);
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.ToString());
                MessageBox.Show(ex.ToString());
                return null;
            }
        }

        public static void DestroyGameObject(uint gameObjectId)
        {
            DLLSystem.CallDLLFunc(() => GameObjectSystem_DestroyGameObject(gameObjectId));
        }

        public static void DestroyDeadGameObjects()
        {
            DLLSystem.CallDLLFunc(() => GameObjectSystem_DestroyDeadGameObjects());
        }

        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern UInt32 GameObjectSystem_CreateGameObjectBase(vec2 gameObjectPosition, uint parentGameObjectId);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern UInt32 GameObjectSystem_CreateGameObject([MarshalAs(UnmanagedType.LPStr)] String gameObjectJson, vec2 gameObjectPosition, uint parentGameObjectId);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern void GameObjectSystem_CreateGameObjectComponent(uint gameObjectId, ComponentTypeEnum componentType, void* componentData);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern void GameObjectSystem_Update(float deltaTime);
        // [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern GameObject* GameObjectSystem_UpdateGameObject(uint gameObjectIndex);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern IntPtr GameObjectSystem_UpdateGameObjectComponent(uint gameObjectId, ComponentTypeEnum componentType);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern void GameObjectSystem_DestroyGameObject(uint gameObjectId);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern void GameObjectSystem_DestroyDeadGameObjects();
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern Transform2DComponent* GameObjectSystem_UpdateTransform2DComponent(uint gameObjectId);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern GameObject* GameObjectSystem_GetGameObjectList(out size_t returnCount);
    }
}