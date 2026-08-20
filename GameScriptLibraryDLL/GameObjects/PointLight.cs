using GameScriptLibraryDLL.Components;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace GameScriptLibraryDLL.GameObjects
{
    public static class PointLightScript
    {
        [UnmanagedCallersOnly] public static IntPtr Create() => GCHandle.ToIntPtr(GCHandle.Alloc(new PointLight()));
        [UnmanagedCallersOnly] public static void StartUp(IntPtr instancePtr, uint id, uint parent) => GameObject.GetFromPtr<PointLight>(instancePtr).StartUp(instancePtr, id, parent);
        [UnmanagedCallersOnly] public static void Update(IntPtr instancePtr, float dt) => GameObject.GetFromPtr<PointLight>(instancePtr).Update(instancePtr, dt);
        [UnmanagedCallersOnly] public static void OnCollisionEnter(IntPtr instancePtr, uint gameObjectId, uint collidingGameObjectId) => GameObject.GetFromPtr<PointLight>(instancePtr).OnCollisionEnter(instancePtr, gameObjectId, collidingGameObjectId);
        [UnmanagedCallersOnly] public static void OnCollisionStay(IntPtr instancePtr, uint gameObjectId, uint collidingGameObjectId) => GameObject.GetFromPtr<PointLight>(instancePtr).OnCollisionEnter(instancePtr, gameObjectId, collidingGameObjectId);
        [UnmanagedCallersOnly] public static void OnCollisionExit(IntPtr instancePtr, uint gameObjectId, uint collidingGameObjectId) => GameObject.GetFromPtr<PointLight>(instancePtr).OnCollisionEnter(instancePtr, gameObjectId, collidingGameObjectId);
        [UnmanagedCallersOnly] public static void Destroy(IntPtr instancePtr) => GameObject.GetFromPtr<PointLight>(instancePtr).Destroy(instancePtr);
    }

    public unsafe class PointLight : GameObject, IGameObjectType
    {
        public static GameObjectTypeEnum ObjectType => GameObjectTypeEnum.kGameObjectPointLight;

        public override IntPtr Create()
        {
            var instance = new PointLight();
            GCHandle handle = GCHandle.Alloc(instance, GCHandleType.Normal);
            return GCHandle.ToIntPtr(handle);
        }

        public override void StartUp(IntPtr instancePtr, uint gameObjectId, uint parentGameObjectId)
        {
        }

        public override void OnCollisionEnter(IntPtr instancePtr, uint gameObjectId, uint collidingGameObjectId)
        {
        }

        public override void OnCollisionStay(IntPtr instancePtr, uint gameObjectId, uint collidingGameObjectId)
        {
        }

        public override void OnCollisionExit(IntPtr instancePtr, uint gameObjectId, uint collidingGameObjectId)
        {
        }

        public override void Update(IntPtr instancePtr, float deltaTime)
        {

        }

        public override void Destroy(IntPtr instance)
        {
            if (instance != IntPtr.Zero)
            {
                GCHandle handle = GCHandle.FromIntPtr(instance);
                handle.Free();
            }
        }
    }
}
