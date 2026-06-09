using GameScriptLibraryDLL.Components;
using GlmSharp;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace GameScriptLibraryDLL.GameObjects
{
    public interface IGameObject
    {
        public abstract IntPtr Create();
        public abstract void StartUp(IntPtr instancePtr, uint gameObjectId, uint parentGameObjectId);
        public abstract void OnCollisionEnter(IntPtr instancePtr, uint gameObjectId, uint collidingGameObjectId);
        public abstract void OnCollisionExit(IntPtr instancePtr, uint gameObjectId, uint collidingGameObjectId);
        public abstract void Update(IntPtr instancePtr, float deltaTime);
        public abstract void Destroy(IntPtr instance);
    }

    public unsafe class GameObject
    {
        public uint ParentGameObjectId { get; protected set; } = uint.MaxValue;
        public uint GameObjectId { get; protected set; } = uint.MaxValue;

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

        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern GameObjectVariableDLL* GameObjectSystem_GetGameObjectVariables(uint gameObjectId, out nuint returnCount);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern UInt32 GameObjectSystem_CreateGameObjectBase(vec2 gameObjectPosition, uint parentGameObjectId);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern UInt32 GameObjectSystem_CreateGameObject(GameObjectTypeEnum gameObjectType, vec2 gameObjectPosition, uint parentGameObjectId);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern intptr_t GameObjectSystem_GetGameObjectPtr(uint gameObjectId);
    }
}
