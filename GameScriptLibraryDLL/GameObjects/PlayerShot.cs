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
    public static class PlayerShotScript
    {
        [UnmanagedCallersOnly] public static IntPtr Create() => GCHandle.ToIntPtr(GCHandle.Alloc(new PlayerShot()));
        [UnmanagedCallersOnly] public static void StartUp(IntPtr instancePtr, uint id, uint parent) => GameObject.GetFromPtr<PlayerShot>(instancePtr).StartUp(instancePtr, id, parent);
        [UnmanagedCallersOnly] public static void Update(IntPtr instancePtr, float dt) => GameObject.GetFromPtr<PlayerShot>(instancePtr).Update(instancePtr, dt);
        [UnmanagedCallersOnly] public static void OnCollisionEnter(IntPtr instancePtr, uint gameObjectId, uint collidingGameObjectId) => GameObject.GetFromPtr<PlayerShot>(instancePtr).OnCollisionEnter(instancePtr, gameObjectId, collidingGameObjectId);
        [UnmanagedCallersOnly] public static void OnCollisionStay(IntPtr instancePtr, uint gameObjectId, uint collidingGameObjectId) => GameObject.GetFromPtr<PlayerShot>(instancePtr).OnCollisionEnter(instancePtr, gameObjectId, collidingGameObjectId);
        [UnmanagedCallersOnly] public static void OnCollisionExit(IntPtr instancePtr, uint gameObjectId, uint collidingGameObjectId) => GameObject.GetFromPtr<PlayerShot>(instancePtr).OnCollisionEnter(instancePtr, gameObjectId, collidingGameObjectId);
        [UnmanagedCallersOnly] public static void Destroy(IntPtr instancePtr) => GameObject.GetFromPtr<PlayerShot>(instancePtr).Destroy(instancePtr);
    }

    public unsafe class PlayerShot : GameObject, IGameObjectType
    {
        public float Speed { get; } = 200.0f;
        public static GameObjectTypeEnum ObjectType => GameObjectTypeEnum.kGameObjectMegaManShot;

        public override IntPtr Create()
        {
            var instance = new PlayerShot();
            GCHandle handle = GCHandle.Alloc(instance, GCHandleType.Normal);
            return GCHandle.ToIntPtr(handle);
        }

        public override void StartUp(IntPtr instancePtr, uint gameObjectId, uint parentGameObjectId)
        {
            if (instancePtr == IntPtr.Zero) return;

            var instance = GameObject.GetFromPtr<PlayerShot>(instancePtr);
            instance.ParentGameObjectId = parentGameObjectId;
            instance.GameObjectId = gameObjectId;

            Player player = GameObject.GetById<Player>(instance.ParentGameObjectId);
            player.PlayerShotCount++;
        }

        public override void OnCollisionEnter(IntPtr instancePtr, uint gameObjectId, uint collidingGameObjectId)
        {
            if (instancePtr == IntPtr.Zero) return;
            var instance = GameObject.GetFromPtr<PlayerShot>(instancePtr);
            var parentGameObject = GameObject.GetById<GameObject>(gameObjectId);
            var hitGameObject = GameObject.GetById<GameObject>(collidingGameObjectId);
            if (parentGameObject.GameObjectId == hitGameObject.GameObjectId) return;

            Console.WriteLine("[Player Shot Object] Object has entered collision zone.");
        }

        public override void OnCollisionStay(IntPtr instancePtr, uint gameObjectId, uint collidingGameObjectId)
        {
            Console.WriteLine("[Player Shot Object] Object is still in collision zone.");
        }

        public override void OnCollisionExit(IntPtr instancePtr, uint gameObjectId, uint collidingGameObjectId)
        {
            Console.WriteLine("[Player Shot Object] Object has exited collision zone.");
        }

        public override void Update(IntPtr instancePtr, float deltaTime)
        {
            if (instancePtr == IntPtr.Zero) return;

            var instance = (PlayerShot)GCHandle.FromIntPtr(instancePtr).Target;
            Transform2DComponent* transform = Component.GetGameObjectComponent<Transform2DComponent>(instance.GameObjectId, ComponentTypeEnum.kTransform2DComponent);
            transform->Position = new(transform->Position.x, transform->Position.y + 200.0f * deltaTime);

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
