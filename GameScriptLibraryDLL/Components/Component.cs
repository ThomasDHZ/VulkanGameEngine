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
        kDebugObjectComponent,
        kEndOfEnum
    }

    public unsafe class Component
    {
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

        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern IntPtr GameObjectSystem_UpdateGameObjectComponent(uint gameObjectId, ComponentTypeEnum componentType);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern void GameObjectSystem_CreateGameObjectComponent(uint gameObjectId, ComponentTypeEnum componentType, void* componentData);
    }
}
