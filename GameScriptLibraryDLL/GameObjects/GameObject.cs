using GameScriptLibraryDLL.Components;
using GlmSharp;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using VulkanEngineCoreCS;

namespace GameScriptLibraryDLL.GameObjects
{
    public interface IGameObject
    {
        public IntPtr Create();
        public void StartUp(IntPtr instancePtr, uint gameObjectId, uint parentGameObjectId);
        public void OnCollisionEnter(IntPtr instancePtr, uint gameObjectId, uint collidingGameObjectId);
        public void OnCollisionStay(IntPtr instancePtr, uint gameObjectId, uint collidingGameObjectId);
        public void OnCollisionExit(IntPtr instancePtr, uint gameObjectId, uint collidingGameObjectId);
        public void Update(IntPtr instancePtr, float deltaTime);
        public void Destroy(IntPtr instance);
    }

    public unsafe class GameObject : IGameObject
    {
        public uint ParentGameObjectId;
        public uint GameObjectId;
        public IntPtr GameObjectPtr = IntPtr.Zero;
        public virtual GameObjectTypeEnum ObjectType { get; protected set; } = GameObjectTypeEnum.kGameObjectNone;
        public bool GameObjectAlive = true;

        public GameObject() { }

        public static UInt32 CreateGameObject(GameObjectTypeEnum gameObjectType, vec2 gameObjectPosition, uint parentGameObjectId = uint.MaxValue)
        {
            return DLLSystem.CallDLLFunc(() => GameObjectSystem_CreateGameObject(gameObjectType, gameObjectPosition, parentGameObjectId));
        }

        //public static Dictionary<string, GameObjectVariable<T>> GetGameObjectVariables<T>(uint gameObjectId)
        //{
        //    //try
        //    //{
        //    //    var gameObjectVariableMap = new Dictionary<string, GameObjectVariable<T>>();
        //    //    GameObjectVariableDLL* listPtr = GameObjectSystem_GetGameObjectVariables(gameObjectId, out nuint count);

        //    //    if (listPtr == null || count == 0) return gameObjectVariableMap;
        //    //    for (nuint x = 0; x < count; x++)
        //    //    {
        //    //        GameObjectVariableDLL raw = listPtr[x];
        //    //        string name = Marshal.PtrToStringAnsi(raw.VariableName);
        //    //        if (string.IsNullOrEmpty(name)) continue;

        //    //        var variable = new GameObjectVariable<T>
        //    //        {
        //    //            VariableName = name,
        //    //            Value = *raw.GetAs<T>(),
        //    //            ValueByteSize = raw.ValueByteSize,
        //    //            MemberTypeEnum = raw.MemberTypeEnum,
        //    //            ConstVariable = raw.ConstVariable
        //    //        };
        //    //        gameObjectVariableMap[name] = variable;
        //    //        MemorySystem.RemovePtrBuffer((IntPtr)raw.ValuePtr);
        //    //        MemorySystem.RemovePtrBuffer((IntPtr)raw.VariableName);
        //    //    }
        //    //    MemorySystem.RemovePtrBuffer((IntPtr)listPtr);
        //    //    return gameObjectVariableMap;
        //    //}
        //    //catch (Exception ex)
        //    //{
        //    //    Console.WriteLine(ex.ToString());
        //    //    return null;
        //    //}
        //}

        public virtual IntPtr Create()
        {
            return IntPtr.Zero;
        }

        public virtual void StartUp(IntPtr instancePtr, uint gameObjectId, uint parentGameObjectId)
        {

        }

        public virtual void OnCollisionEnter(IntPtr instancePtr, uint gameObjectId, uint collidingGameObjectId)
        {

        }

        public virtual void OnCollisionStay(IntPtr instancePtr, uint gameObjectId, uint collidingGameObjectId)
        {
        }

        public virtual void OnCollisionExit(IntPtr instancePtr, uint gameObjectId, uint collidingGameObjectId)
        {
        }

        public virtual void Update(IntPtr instancePtr, float deltaTime)
        {
        }

        public virtual void Destroy(IntPtr instance)
        {
        }

        public static void DestroyGameObject(uint gameObjectId)
        {
            DLLSystem.CallDLLFunc(() => GameObjectSystem_DestroyGameObject(gameObjectId));
        }

        public static T? GetFromPtr<T>(IntPtr instancePtr) where T : class
        {
            if (instancePtr == IntPtr.Zero) return null;
            return GCHandle.FromIntPtr(instancePtr).Target as T;
        }

        public static T? GetById<T>(uint gameObjectId) where T : class
        {
            IntPtr ptr = GameObjectSystem_GetGameObjectPtr(gameObjectId);
            if (ptr == IntPtr.Zero) return null;
            return GetFromPtr<T>(ptr);
        }

        [DllImport(Module.VulkanEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern UInt32 GameObjectSystem_CreateGameObject(GameObjectTypeEnum gameObjectType, vec2 gameObjectPosition, uint parentGameObjectId);
        [DllImport(Module.VulkanEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern IntPtr GameObjectSystem_GetGameObjectPtr(uint gameObjectId);
        [DllImport(Module.VulkanEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern void GameObjectSystem_DestroyGameObject(uint gameObjectId);
    }
}
