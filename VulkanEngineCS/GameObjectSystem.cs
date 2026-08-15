using GlmSharp;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using VulkanEngineCoreCS;

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

    public enum ComponentTypeEnum : UInt64
    {
        kInputComponent,
        kSpriteComponent,
        kTransform2DComponent,
        kTransform3DComponent,
        kCameraFollowComponent,
        kDirectionalLightComponent,
        kPointLightComponent,
        kDebugObjectComponent,
        kCollisionComponent,
        kEndOfEnum
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

    public unsafe class GameObjectSystem
    {
        public static void Update(float deltaTime)
        {
            DLLSystem.CallDLLFunc(() => GameObjectSystem_Update(deltaTime));
        }

        public static void CreateGameObject(GameObjectTypeEnum gameObjectJson, vec2 gameObjectPosition, uint parentGameObjectId = uint.MaxValue)
        { 
            DLLSystem.CallDLLFunc(() => GameObjectSystem_CreateGameObject(gameObjectJson, gameObjectPosition, parentGameObjectId));
        }

        //public static GameObjectVariableDLL* GetGameObjectVariables(uint gameObjectId)
        //{
        //    return DLLSystem.CallDLLFunc(() => GameObjectSystem_GetGameObjectVariables(gameObjectId, gameObjectPosition, parentGameObjectId));
        //}

        public static IntPtr GetGameObjectPtr(uint gameObjectId)
        {
            return DLLSystem.CallDLLFunc(() => GameObjectSystem_GetGameObjectPtr(gameObjectId));
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
        [DllImport("VulkanEngineInterop.dll", CallingConvention = CallingConvention.Cdecl)] private static extern void GameObjectSystem_Update(float deltaTime);
        [DllImport("VulkanEngineInterop.dll", CallingConvention = CallingConvention.Cdecl)] private static extern void GameObjectSystem_CreateGameObject(GameObjectTypeEnum gameObjectJson, vec2 gameObjectPosition, uint parentGameObjectId = uint.MaxValue);
        [DllImport("VulkanEngineInterop.dll", CallingConvention = CallingConvention.Cdecl)] private static extern GameObjectVariableDLL* GameObjectSystem_GetGameObjectVariables(uint gameObjectId, out size_t returnCount);
        [DllImport("VulkanEngineInterop.dll", CallingConvention = CallingConvention.Cdecl)] private static extern IntPtr GameObjectSystem_GetGameObjectPtr(uint gameObjectId);
        [DllImport("VulkanEngineInterop.dll", CallingConvention = CallingConvention.Cdecl)] private static extern void GameObjectSystem_DestroyGameObject(uint gameObjectId);
        [DllImport("VulkanEngineInterop.dll", CallingConvention = CallingConvention.Cdecl)] private static extern void GameObjectSystem_CreateGameObjectComponent(uint gameObjectId, ComponentTypeEnum componentType, void* componentData);
        [DllImport("VulkanEngineInterop.dll", CallingConvention = CallingConvention.Cdecl)] private static extern IntPtr GameObjectSystem_UpdateGameObjectComponent(uint gameObjectId, ComponentTypeEnum componentType);
    }
}
