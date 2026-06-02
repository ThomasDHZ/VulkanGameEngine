using GameScriptLibraryDLL.GameObjects;
using GlmSharp;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using static GameScriptLibraryDLL.GameObjects.GameObjectVariableDLL;

namespace GameScriptLibraryDLL.Components
{
    public enum ComponentTypeEnum : uint
    {
        kInputComponent,
        kSpriteComponent,
        kTransform2DComponent,
        kTransform3DComponent,
        kCameraFollowComponent,
        kDirectionalLightComponent,
        kPointLightComponent,
        kEndOfEnum
    }

    public unsafe class Component
    {
        public static UInt32 CreateGameObject(GameObjectTypeEnum gameObjectType, vec2 gameObjectPosition, uint parentGameObjectId = uint.MaxValue)
        {
            return DLLSystem.CallDLLFunc(() => GameObjectSystem_CreateGameObject(gameObjectType, gameObjectPosition, parentGameObjectId));
        }

        public static T* GetGameObjectComponent<T>(uint gameObjectId, ComponentTypeEnum componentType) where T : struct
        {
            IntPtr ptr = GameObjectSystem_UpdateGameObjectComponent(gameObjectId, componentType);
            if (ptr == IntPtr.Zero)
                return null;

            return (T*)ptr.ToPointer();
        }

        public static unsafe void CreateGameObjectComponent<T>(uint gameObjectId, ComponentTypeEnum componentType, ref T componentData) where T : unmanaged
        {
            fixed (T* pComponent = &componentData)
            {
                GameObjectSystem_CreateGameObjectComponent(gameObjectId, componentType, pComponent);
            }
        }

        public static Dictionary<string, GameObjectVariable<T>> GetGameObjectVariables<T>(uint gameObjectId) 
        {
            try
            {
                var gameObjectVariableMap = new Dictionary<string, GameObjectVariable<T>>();
                GameObjectVariableDLL* listPtr = GameObjectSystem_GetGameObjectVariables(gameObjectId, out nuint count);

                if (listPtr == null || count == 0) return gameObjectVariableMap;
                for (nuint x = 0; x < count; x++)
                {
                    GameObjectVariableDLL raw = listPtr[x];
                    string name = Marshal.PtrToStringAnsi(raw.VariableName);
                    if (string.IsNullOrEmpty(name)) continue;

                    var variable = new GameObjectVariable<T>
                    {
                        VariableName = name,
                        Value = *raw.GetAs<T>(),
                        ValueByteSize = raw.ValueByteSize,
                        MemberTypeEnum = raw.MemberTypeEnum,
                        ConstVariable = raw.ConstVariable
                    };
                    gameObjectVariableMap[name] = variable;
                    MemorySystem.RemovePtrBuffer((IntPtr)raw.ValuePtr);
                    MemorySystem.RemovePtrBuffer((IntPtr)raw.VariableName);
                }
                MemorySystem.RemovePtrBuffer((IntPtr)listPtr);
                return gameObjectVariableMap;
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.ToString());
                return null;
            }
        }

        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern GameObjectVariableDLL* GameObjectSystem_GetGameObjectVariables(uint gameObjectId, out nuint returnCount);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern IntPtr GameObjectSystem_UpdateGameObjectComponent(uint gameObjectId, ComponentTypeEnum componentType);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern void GameObjectSystem_CreateGameObjectComponent(uint gameObjectId, ComponentTypeEnum componentType, void* componentData);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern UInt32 GameObjectSystem_CreateGameObjectBase(vec2 gameObjectPosition, uint parentGameObjectId);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern UInt32 GameObjectSystem_CreateGameObject(GameObjectTypeEnum gameObjectType, vec2 gameObjectPosition, uint parentGameObjectId);

    }
}
