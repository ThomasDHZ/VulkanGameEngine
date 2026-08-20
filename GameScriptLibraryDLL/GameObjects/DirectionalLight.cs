using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace GameScriptLibraryDLL.GameObjects
{
    public static class DirectionalLightScript
    {
        [UnmanagedCallersOnly] public static IntPtr Create() => GCHandle.ToIntPtr(GCHandle.Alloc(new DirectionalLight()));
        [UnmanagedCallersOnly] public static void StartUp(IntPtr instancePtr, uint id, uint parent) => GameObject.GetFromPtr<DirectionalLight>(instancePtr).StartUp(instancePtr, id, parent);
        [UnmanagedCallersOnly] public static void Update(IntPtr instancePtr, float dt) => GameObject.GetFromPtr<DirectionalLight>(instancePtr).Update(instancePtr, dt);
        [UnmanagedCallersOnly] public static void OnCollisionEnter(IntPtr instancePtr, uint gameObjectId, uint collidingGameObjectId) => GameObject.GetFromPtr<DirectionalLight>(instancePtr).OnCollisionEnter(instancePtr, gameObjectId, collidingGameObjectId);
        [UnmanagedCallersOnly] public static void OnCollisionStay(IntPtr instancePtr, uint gameObjectId, uint collidingGameObjectId) => GameObject.GetFromPtr<DirectionalLight>(instancePtr).OnCollisionEnter(instancePtr, gameObjectId, collidingGameObjectId);
        [UnmanagedCallersOnly] public static void OnCollisionExit(IntPtr instancePtr, uint gameObjectId, uint collidingGameObjectId) => GameObject.GetFromPtr<DirectionalLight>(instancePtr).OnCollisionEnter(instancePtr, gameObjectId, collidingGameObjectId);
        [UnmanagedCallersOnly] public static void Destroy(IntPtr instancePtr) => GameObject.GetFromPtr<DirectionalLight>(instancePtr).Destroy(instancePtr);
    }

    public class DirectionalLight : GameObject, IGameObjectType
    {
        public static GameObjectTypeEnum ObjectType => GameObjectTypeEnum.kGameObjectDirectionalLight;

        public override IntPtr Create()
        {
            var instance = new DirectionalLight();
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
