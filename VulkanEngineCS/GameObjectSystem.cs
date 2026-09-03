using GameScriptLibraryDLL.Components;
using GameScriptLibraryDLL.GameObjects;
using GlmSharp;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using VulkanEngineCoreCS;
using VulkanEngineCoreCS.Vulkan;

namespace VulkanEngineCS
{
    enum GameObjectMemberType
    {
        GameObjectVarUnknown,
        GameObjectVarInt,
        GameObjectVarUint,
        GameObjectVarFloat,
        GameObjectVarIvec2,
        GameObjectVarIvec3,
        GameObjectVarIvec4,
        GameObjectVarVec2,
        GameObjectVarVec3,
        GameObjectVarVec4,
        GameObjectVarMat2,
        GameObjectVarMat3,
        GameObjectVarMat4,
        GameObjectVarBool
    };
    public enum GameObjectTypeEnum
    {
        kGameObjectNone,
        kGameObjectMegaMan,
        kGameObjectMegaManShot,
        kGameObjectDirectionalLight,
        kGameObjectPointLight,
        kGameObjectEnemy
    };

    public unsafe struct GameObjectVariableDLL
    {
        String VariableName;
        byte* Value;
        size_t VariableByteSize = 0;
        GameObjectMemberType MemberTypeEnum;
        bool ConstVariable = false;

        public GameObjectVariableDLL()
        {
        }
    };

    [StructLayout(LayoutKind.Sequential)]
    public struct GameObjecLevelEditor
    {
        public uint ParentGameObjectId;
        public uint GameObjectId;
        public IntPtr GameObjectPtr;
        public GameObjectTypeEnum GameObjectType;
        [MarshalAs(UnmanagedType.I1)]
        public bool GameObjectAlive;
    }

    public unsafe class GameObjectSystem
    {
        public static void Update(float deltaTime)
        {
            DLLSystem.CallDLLFunc(() => GameObjectSystem_Update(deltaTime));
        }

        public static uint CreateGameObject(GameObjectTypeEnum gameObjectJson, vec2 gameObjectPosition, uint parentGameObjectId = uint.MaxValue)
        {
            return DLLSystem.CallDLLFunc(() => GameObjectSystem_CreateGameObject(gameObjectJson, gameObjectPosition, parentGameObjectId));
        }

        public static uint CreateGameObject(string gameObjectJson, vec2 gameObjectPosition, uint parentGameObjectId = uint.MaxValue)
        {
            return DLLSystem.CallDLLFunc(() => GameObjectSystem_CreateGameObjectJson(gameObjectJson, gameObjectPosition, parentGameObjectId));
        }

        //public static GameObjectVariableDLL* GetGameObjectVariables(uint gameObjectId)
        //{
        //    return DLLSystem.CallDLLFunc(() => GameObjectSystem_GetGameObjectVariables(gameObjectId, gameObjectPosition, parentGameObjectId));
        //}

        public static unsafe GameObjecLevelEditor* GetGameObject(uint gameObjectId)
        {
            GameObjecLevelEditor* ptr = GameObjectSystem_GetGameObject(gameObjectId);
            return ptr;
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

        public static unsafe List<GameObjecLevelEditor> GetGameObjectList()
        {
            try
            {
                IntPtr ptr = GameObjectSystem_GetGameObjectList(out nuint count);
                if (ptr == IntPtr.Zero || count == 0)
                {
                    return new List<GameObjecLevelEditor>();
                }

                var list = (GameObjecLevelEditor*)ptr;
                var result = new List<GameObjecLevelEditor>((int)count);
                for (int i = 0; i < (int)count; i++)
                {
                    result.Add(list[i]);
                }

                MemorySystem.DeletePtr((GameObjecLevelEditor*)ptr.ToPointer());
                return result;
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.ToString());
                return null;
            }
        }

        public static void DestroyGameObject(uint gameObjectId)
        {
            DLLSystem.CallDLLFunc(() => GameObjectSystem_DestroyGameObject(gameObjectId));
        }

        public static void CreateGameObjectComponent(uint gameObjectId, ComponentTypeEnum componentType, void* componentData)
        {
            DLLSystem.CallDLLFunc(() => GameObjectSystem_CreateGameObjectComponent(gameObjectId, componentType, componentData));
        }

        public static IntPtr UpdateGameObjectComponent(uint gameObjectId, ComponentTypeEnum componentType)
        {
            return DLLSystem.CallDLLFunc(() => GameObjectSystem_UpdateGameObjectComponent(gameObjectId, componentType));
        }

        public static unsafe ref T UpdateGameObjectComponent<T>(uint gameObjectId, ComponentTypeEnum componentType) where T : unmanaged
        {
            IntPtr ptr = DLLSystem.CallDLLFunc(() =>
                GameObjectSystem_UpdateGameObjectComponent(gameObjectId, componentType));

            if (ptr == IntPtr.Zero)
                throw new InvalidOperationException($"No {typeof(T).Name} on {gameObjectId}");

            return ref *(T*)ptr;
        }

        public static List<ComponentTypeEnum> GetGameObjectComponentList(uint gameObjectId)
        {
            try
            {
                ComponentTypeEnum* gameObjectComponentListPtr = GameObjectSystem_GetGameObjectComponentList(gameObjectId, out size_t gameObjectComopnentCount);
                List<ComponentTypeEnum> componentTypeEnumList = new ListPtr<ComponentTypeEnum>(gameObjectComponentListPtr, gameObjectComopnentCount).ToList();
                return componentTypeEnumList;
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.ToString());
                return null;
            }
        }

        [DllImport("VulkanEngineInterop.dll", CallingConvention = CallingConvention.Cdecl)] private static extern void GameObjectSystem_Update(float deltaTime);
        [DllImport("VulkanEngineInterop.dll", CallingConvention = CallingConvention.Cdecl)] private static extern uint GameObjectSystem_CreateGameObject(GameObjectTypeEnum gameObjectJson, vec2 gameObjectPosition, uint parentGameObjectId = uint.MaxValue);
        [DllImport("VulkanEngineInterop.dll", CallingConvention = CallingConvention.Cdecl)] private static extern uint GameObjectSystem_CreateGameObjectJson([MarshalAs(UnmanagedType.LPStr)] string gameObjectJson, vec2 gameObjectPosition, uint parentGameObjectId);
        [DllImport("VulkanEngineInterop.dll", CallingConvention = CallingConvention.Cdecl)] private static extern GameObjectVariableDLL* GameObjectSystem_GetGameObjectVariables(uint gameObjectId, out size_t returnCount);
        [DllImport("VulkanEngineInterop.dll", CallingConvention = CallingConvention.Cdecl)] private static extern GameObjecLevelEditor* GameObjectSystem_GetGameObject(uint gameObjectId);
        [DllImport("VulkanEngineInterop.dll", CallingConvention = CallingConvention.StdCall)] private static extern IntPtr GameObjectSystem_GetGameObjectList(out size_t returnCount);
        [DllImport("VulkanEngineInterop.dll", CallingConvention = CallingConvention.Cdecl)] private static extern void GameObjectSystem_DestroyGameObject(uint gameObjectId);
        [DllImport("VulkanEngineInterop.dll", CallingConvention = CallingConvention.Cdecl)] private static extern void GameObjectSystem_CreateGameObjectComponent(uint gameObjectId, ComponentTypeEnum componentType, void* componentData);
        [DllImport("VulkanEngineInterop.dll", CallingConvention = CallingConvention.StdCall)] private static extern IntPtr GameObjectSystem_UpdateGameObjectComponent(uint gameObjectId, ComponentTypeEnum componentType);
        [DllImport("VulkanEngineInterop.dll", CallingConvention = CallingConvention.Cdecl)] private static extern ComponentTypeEnum* GameObjectSystem_GetGameObjectComponentList(size_t gameObjectId, out size_t returnCount);
    }
}
