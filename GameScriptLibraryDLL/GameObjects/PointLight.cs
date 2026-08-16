using GameScriptLibraryDLL.Components;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace GameScriptLibraryDLL.GameObjects
{
    public unsafe class PointLight : GameObject, IGameObjectType
    {
        public static GameObjectTypeEnum ObjectType => GameObjectTypeEnum.kGameObjectPointLight;

        [UnmanagedCallersOnly]
        public static IntPtr Create()
        {
            var instance = new PointLight();
            GCHandle handle = GCHandle.Alloc(instance, GCHandleType.Normal);
            return GCHandle.ToIntPtr(handle);
        }

        [UnmanagedCallersOnly]
        public static void StartUp(IntPtr instancePtr, uint gameObjectId, uint parentGameObjectId)
        {
        }

        [UnmanagedCallersOnly]
        public static void OnCollisionEnter(IntPtr instancePtr, uint gameObjectId, uint collidingGameObjectId)
        {
        }

        [UnmanagedCallersOnly]
        public static void OnCollisionStay(IntPtr instancePtr, uint gameObjectId, uint collidingGameObjectId)
        {
        }

        [UnmanagedCallersOnly]
        public static void OnCollisionExit(IntPtr instancePtr, uint gameObjectId, uint collidingGameObjectId)
        {
        }

        [UnmanagedCallersOnly]
        public static void Update(IntPtr instancePtr, float deltaTime)
        {

        }

        [UnmanagedCallersOnly]
        public static void Destroy(IntPtr instance)
        {
            if (instance != IntPtr.Zero)
            {
                GCHandle handle = GCHandle.FromIntPtr(instance);
                handle.Free();
            }
        }
    }
}
