using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
//using VulkanGameEngineLevelEditor.GameEngine;
//using VulkanGameEngineLevelEditor.GameEngine.Components;

namespace GameScriptLibraryDLL
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
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern IntPtr GameObjectSystem_UpdateGameObjectComponent(uint gameObjectId, ComponentTypeEnum componentType);

    }
}
