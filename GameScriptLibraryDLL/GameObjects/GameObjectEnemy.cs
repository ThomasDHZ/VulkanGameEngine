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
    public static class GameObjectEnemyScript
    {
        [UnmanagedCallersOnly] public static IntPtr Create() => GCHandle.ToIntPtr(GCHandle.Alloc(new GameObjectEnemy()));
        [UnmanagedCallersOnly] public static void StartUp(IntPtr instancePtr, uint id, uint parent) => GameObject.GetFromPtr<GameObjectEnemy>(instancePtr).StartUp(instancePtr, id, parent);
        [UnmanagedCallersOnly] public static void Update(IntPtr instancePtr, float dt) => GameObject.GetFromPtr<GameObjectEnemy>(instancePtr).Update(instancePtr, dt);
        [UnmanagedCallersOnly] public static void OnCollisionEnter(IntPtr instancePtr, uint gameObjectId, uint collidingGameObjectId) => GameObject.GetFromPtr<GameObjectEnemy>(instancePtr).OnCollisionEnter(instancePtr, gameObjectId, collidingGameObjectId);
        [UnmanagedCallersOnly] public static void OnCollisionStay(IntPtr instancePtr, uint gameObjectId, uint collidingGameObjectId) => GameObject.GetFromPtr<GameObjectEnemy>(instancePtr).OnCollisionEnter(instancePtr, gameObjectId, collidingGameObjectId);
        [UnmanagedCallersOnly] public static void OnCollisionExit(IntPtr instancePtr, uint gameObjectId, uint collidingGameObjectId) => GameObject.GetFromPtr<GameObjectEnemy>(instancePtr).OnCollisionEnter(instancePtr, gameObjectId, collidingGameObjectId);
        [UnmanagedCallersOnly] public static void Destroy(IntPtr instancePtr) => GameObject.GetFromPtr<GameObjectEnemy>(instancePtr).Destroy(instancePtr);
    }

    public class GameObjectEnemy : GameObject, IGameObjectType
    {
        public uint Hp { get; set; } = 5;
        public override IntPtr Create()
        {
            var instance = new GameObjectEnemy();
            GCHandle handle = GCHandle.Alloc(instance, GCHandleType.Normal);
            return GCHandle.ToIntPtr(handle);
        }

        public override void StartUp(IntPtr instancePtr, uint gameObjectId, uint parentGameObjectId)
        {
            if (instancePtr == IntPtr.Zero) return;

            var instance = GameObject.GetFromPtr<GameObjectEnemy>(instancePtr);
            instance.ParentGameObjectId = parentGameObjectId;
            instance.GameObjectId = gameObjectId;
        }

        public override void OnCollisionEnter(IntPtr instancePtr, uint gameObjectId, uint collidingGameObjectId)
        {
            if(GameObject.GetById<PlayerShot>(collidingGameObjectId) is PlayerShot shot)
            {
                Hp -= shot.Power;
                shot.Destroy(shot.GameObjectPtr);
            }
            Console.WriteLine("[Enemy Object] Object has entered collision zone.");
        }

        public override void OnCollisionStay(IntPtr instancePtr, uint gameObjectId, uint collidingGameObjectId)
        {
            Console.WriteLine("[Enemy Object] Object is still in collision zone.");
        }

        public override void OnCollisionExit(IntPtr instancePtr, uint gameObjectId, uint collidingGameObjectId)
        {
            Console.WriteLine("[Enemy Object] Object has exited collision zone.");
        }

        public override void Update(IntPtr instancePtr, float deltaTime)
        {
            if (Hp <= 0) Destroy(instancePtr);
        }

        public override void Destroy(IntPtr instancePtr)
        {
            if (instancePtr != IntPtr.Zero)
            {
                GCHandle handle = GCHandle.FromIntPtr(instancePtr);
                handle.Free();
            }
        }
    }
}
