using GlmSharp;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace GameScriptLibraryDLL
{
    public unsafe class PlayerShot
    {
        public Guid VrameId { get; } = new Guid("623e5b6b-b1f8-4e69-8dca-237069a373e2");
        public uint ComponentType { get; } = (uint)(ComponentTypeEnum.kTransform2DComponent | ComponentTypeEnum.kSpriteComponent);
        public float Speed { get; } = 200.0f;

        [UnmanagedCallersOnly]
        public static IntPtr Create()
        {
            var instance = new Player();
            GCHandle handle = GCHandle.Alloc(instance, GCHandleType.Normal);
            return GCHandle.ToIntPtr(handle);
        }

        [UnmanagedCallersOnly]
        public static void StartUp(IntPtr instancePtr, uint gameObjectId)
        {
            if (instancePtr == IntPtr.Zero) return;
            ComponentRegistery.Create(gameObjectId, ComponentTypeEnum.kTransform2DComponent);
            ComponentRegistery.Create(gameObjectId, ComponentTypeEnum.kSpriteComponent);
        }

        [UnmanagedCallersOnly]
        public static void Update(IntPtr instancePtr, float deltaTime)
        {
            if (instancePtr == IntPtr.Zero) return;

            var instance = (Player)GCHandle.FromIntPtr(instancePtr).Target;
            Transform2DComponent* transform = Component.GetGameObjectComponent<Transform2DComponent>(instance.GameObjectId, ComponentTypeEnum.kTransform2DComponent);
            transform->Position = new(transform->Position.x + 200.0f * deltaTime, transform->Position.y);
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
