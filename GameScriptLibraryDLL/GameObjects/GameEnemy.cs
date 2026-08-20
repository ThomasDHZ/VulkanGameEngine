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
    public static class GameEnemyScript
    {
        [UnmanagedCallersOnly] public static IntPtr Create() => GCHandle.ToIntPtr(GCHandle.Alloc(new GameEnemy()));
        [UnmanagedCallersOnly] public static void StartUp(IntPtr instancePtr, uint id, uint parent) => GameObject.GetFromPtr<GameEnemy>(instancePtr).StartUp(instancePtr, id, parent);
        [UnmanagedCallersOnly] public static void Update(IntPtr instancePtr, float dt) => GameObject.GetFromPtr<GameEnemy>(instancePtr).Update(instancePtr, dt);
        [UnmanagedCallersOnly] public static void OnCollisionEnter(IntPtr instancePtr, uint gameObjectId, uint collidingGameObjectId) => GameObject.GetFromPtr<GameEnemy>(instancePtr).OnCollisionEnter(instancePtr, gameObjectId, collidingGameObjectId);
        [UnmanagedCallersOnly] public static void OnCollisionStay(IntPtr instancePtr, uint gameObjectId, uint collidingGameObjectId) => GameObject.GetFromPtr<GameEnemy>(instancePtr).OnCollisionEnter(instancePtr, gameObjectId, collidingGameObjectId);
        [UnmanagedCallersOnly] public static void OnCollisionExit(IntPtr instancePtr, uint gameObjectId, uint collidingGameObjectId) => GameObject.GetFromPtr<GameEnemy>(instancePtr).OnCollisionEnter(instancePtr, gameObjectId, collidingGameObjectId);
        [UnmanagedCallersOnly] public static void Destroy(IntPtr instancePtr) => GameObject.GetFromPtr<GameEnemy>(instancePtr).Destroy(instancePtr);
    }
    public unsafe class GameEnemy : GameObject, IGameObjectType
    {
        public enum MegaManAnimationEnum
        {
            kStanding,
            kWalking,
            kSlide,
            kJump,
            kClimb,
            kClimbEnd,
            kDamage,
            kShoot,
            kShootWalk,
            kShootJump,
            kClimbShoot
        };

        public static GameObjectTypeEnum ObjectType => GameObjectTypeEnum.kGameObjectEnemy;

        public override IntPtr Create()
        {
            var instance = new GameEnemy();
            GCHandle handle = GCHandle.Alloc(instance, GCHandleType.Normal);
            return GCHandle.ToIntPtr(handle);
        }

        public override void StartUp(IntPtr instancePtr, uint gameObjectId, uint parentGameObjectId)
        {
            if (instancePtr == IntPtr.Zero) return;

            var instance = GameObject.GetFromPtr<GameEnemy>(instancePtr);
            instance.ParentGameObjectId = parentGameObjectId;
            instance.GameObjectId = gameObjectId;
        }

        public override void OnCollisionEnter(IntPtr instancePtr, uint gameObjectId, uint collidingGameObjectId)
        {
            if (instancePtr == IntPtr.Zero) return;
            var instance = GameObject.GetFromPtr<GameEnemy>(instancePtr);
            var parentGameObject = GameObject.GetById<GameObject>(gameObjectId);
            var hitGameObject = GameObject.GetById<GameObject>(collidingGameObjectId);
          //  DestroyGameObject(instance.GameObjectId);

            Console.WriteLine("[Player Object] Object has entered collision zone.");
        }

        public override void OnCollisionStay(IntPtr instancePtr, uint gameObjectId, uint collidingGameObjectId)
        {
            Console.WriteLine("[Player Object] Object is still in collision zone.");
        }

        [UnmanagedCallersOnly]
        public static void OnCollisionExit(IntPtr instancePtr, uint gameObjectId, uint collidingGameObjectId)
        {
            Console.WriteLine("[Player Object] Object has exited collision zone.");
        }

        public override void Update(IntPtr instancePtr, float deltaTime)
        {
            if (instancePtr == IntPtr.Zero) return;

            var instance = GameObject.GetFromPtr<GameEnemy>(instancePtr);
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
