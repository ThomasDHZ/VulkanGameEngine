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
    public unsafe class PlayerShot : GameObject, IGameObjectType
    {
        public float Speed { get; } = 200.0f;
        public static GameObjectTypeEnum ObjectType => GameObjectTypeEnum.kGameObjectMegaManShot;

        [UnmanagedCallersOnly]
        public static IntPtr Create()
        {
            var instance = new PlayerShot();
            GCHandle handle = GCHandle.Alloc(instance, GCHandleType.Normal);
            return GCHandle.ToIntPtr(handle);
        }

        [UnmanagedCallersOnly]
        public static void StartUp(IntPtr instancePtr, uint gameObjectId, uint parentGameObjectId)
        {
            if (instancePtr == IntPtr.Zero) return;

            var instance = GameObject.GetFromPtr<PlayerShot>(instancePtr);
            instance.ParentGameObjectId = parentGameObjectId;
            instance.GameObjectId = gameObjectId;

            Player player = GameObject.GetById<Player>(instance.ParentGameObjectId);
            player.PlayerShotCount++;
        }

        [UnmanagedCallersOnly]
        public static void OnCollisionEnter(IntPtr instancePtr, uint gameObjectId, uint collidingGameObjectId)
        {
            if (instancePtr == IntPtr.Zero) return;
            var instance = GameObject.GetFromPtr<PlayerShot>(instancePtr);
            var parentGameObject = GameObject.GetById<GameObject>(gameObjectId);
            var hitGameObject = GameObject.GetById<GameObject>(collidingGameObjectId);
            if (parentGameObject.GameObjectId == hitGameObject.GameObjectId) return;

            Console.WriteLine("[Player Shot Object] Object has entered collision zone.");
        }

        [UnmanagedCallersOnly]
        public static void OnCollisionStay(IntPtr instancePtr, uint gameObjectId, uint collidingGameObjectId)
        {
            Console.WriteLine("[Player Shot Object] Object is still in collision zone.");
        }

        [UnmanagedCallersOnly]
        public static void OnCollisionExit(IntPtr instancePtr, uint gameObjectId, uint collidingGameObjectId)
        {
            Console.WriteLine("[Player Shot Object] Object has exited collision zone.");
        }

        [UnmanagedCallersOnly]
        public static void Update(IntPtr instancePtr, float deltaTime)
        {
            if (instancePtr == IntPtr.Zero) return;

            var instance = (PlayerShot)GCHandle.FromIntPtr(instancePtr).Target;
            Transform2DComponent* transform = Component.GetGameObjectComponent<Transform2DComponent>(instance.GameObjectId, ComponentTypeEnum.kTransform2DComponent);
            transform->Position = new(transform->Position.x, transform->Position.y + 200.0f * deltaTime);

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
