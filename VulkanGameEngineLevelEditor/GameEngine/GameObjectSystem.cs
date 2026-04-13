using GlmSharp;
using Newtonsoft.Json;
using Silk.NET.SDL;
using Silk.NET.Vulkan;
using System;
using System.Collections.Generic;
using System.ComponentModel;
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

    public enum ComponentTypeEnum : uint
    {
        kInputComponent,
        kSpriteComponent,
        kTransform2DComponent,
        kTransform3DComponent,
        kCameraFollowComponent,
        kEndOfEnum
    }


    [StructLayout(LayoutKind.Sequential, Pack = 4)]
    public class Transform2DComponent
    {
        [DisplayName("Position 2D")]
        public vec2 GameObjectPosition { get; set; }
        [DisplayName("Rotation 2D")]
        public vec2 GameObjectRotation { get; set; }
        [DisplayName("Scale 2D")]
        public vec2 GameObjectScale { get; set; }
        [IgnoreProperty]
        public bool Dirty { get; set; }

        public Transform2DComponent()
        {
        }
    };

    public struct Transform3DComponent
    {
        public vec3 GameObjectPosition { get; set; } = new vec3(0.0f);
        public vec3 GameObjectRotation { get; set; } = new vec3(0.0f);
        public vec3 GameObjectScale { get; set; } = new vec3(1.0f);
        public bool Dirty { get; set; } = true;

        public Transform3DComponent()
        {
        }
    };

    public struct GameObject
    {
        public uint GameObjectId { get; set; } = uint.MaxValue;
        public uint ParentGameObjectId { get; set; } = uint.MaxValue;
        public bool GameObjectAlive { get; set; } = true;
        public uint Padding { get; set; } //entt::entity GameObjectComponents in C++ side

        public GameObject()
        {
        }
    }

    public unsafe struct GameObjectComponentContainer
    {
        public ComponentTypeEnum ComponentType { get; set; }
        public IntPtr ComponentPtr { get; set; }
    };

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

        public static IntPtr GetGameObjectComponentPtr(uint gameObjectId, ComponentTypeEnum componentType)
        {
            IntPtr ptr = GameObjectSystem_UpdateGameObjectComponent(gameObjectId, componentType);
            if (ptr == IntPtr.Zero)
            {
                System.Diagnostics.Debug.WriteLine($"Warning: Component {componentType} not found for GO {gameObjectId}");
            }
            return ptr;
        }

        public static GameObject* GetGameObject(uint gameObjectId)
        {
            try
            {
                return GameObjectSystem_GetGameObject(gameObjectId);
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.ToString());
                MessageBox.Show(ex.ToString());
                return null;
            }
        }
    
        public static List<GameObject> GetGameObjectList()
        {
            try
            {
                GameObject* gameObjectList = GameObjectSystem_GetGameObjectList(out size_t gameObjectCount);
                return new ListPtr<GameObject>(gameObjectList, gameObjectCount).ToList();
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.ToString());
                MessageBox.Show(ex.ToString());
                return null;
            }
        }

        public static List<ComponentTypeEnum> GetGameObjectComponentList(uint gameObjectId)
        {
            try
            {
                ComponentTypeEnum* gameObjectComponentListPtr = GameObjectSystem_GetGameObjectComponentList(gameObjectId, out size_t gameObjectComopnentCount);
                List<ComponentTypeEnum> componentTypeEnumList = new ListPtr<ComponentTypeEnum>(gameObjectComponentListPtr, gameObjectComopnentCount).ToList();
                MemorySystem.RemovePtrBuffer(gameObjectComponentListPtr);
                return componentTypeEnumList;
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
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern IntPtr GameObjectSystem_UpdateGameObjectComponent(uint gameObjectId, ComponentTypeEnum componentType); [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern void GameObjectSystem_DestroyGameObject(uint gameObjectId);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern void GameObjectSystem_DestroyDeadGameObjects();
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern Transform2DComponent* GameObjectSystem_UpdateTransform2DComponent(uint gameObjectId);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern GameObject* GameObjectSystem_GetGameObject(uint gameObjectId);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern GameObject* GameObjectSystem_GetGameObjectList(out size_t returnCount);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern ComponentTypeEnum* GameObjectSystem_GetGameObjectComponentList(uint gameObjectId, out size_t returnCount);
    }
}