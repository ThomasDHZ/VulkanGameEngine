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
    public class GameEnemy : GameObject
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

        public override GameObjectTypeEnum ObjectType => GameObjectTypeEnum.kGameObjectEnemy;


        [UnmanagedCallersOnly]
        public static IntPtr Create()
        {
            var instance = new GameEnemy();
            GCHandle handle = GCHandle.Alloc(instance, GCHandleType.Normal);
            return GCHandle.ToIntPtr(handle);
        }

        [UnmanagedCallersOnly]
        public static void StartUp(IntPtr instancePtr, uint gameObjectId, uint parentGameObjectId)
        {
            if (instancePtr == IntPtr.Zero) return;

            var instance = GameObject.GetFromPtr<GameEnemy>(instancePtr);
            instance.ParentGameObjectId = parentGameObjectId;
            instance.GameObjectId = gameObjectId;
        }


        [UnmanagedCallersOnly]
        public static void OnCollisionEnter(IntPtr instancePtr, uint gameObjectId, uint collidingGameObjectId)
        {
            if (instancePtr == IntPtr.Zero) return;
            var instance = GameObject.GetFromPtr<GameEnemy>(instancePtr);
            var parentGameObject = GameObject.GetById<GameObject>(gameObjectId);
            var hitGameObject = GameObject.GetById<GameObject>(collidingGameObjectId);
            DestroyGameObject(instance.GameObjectId);

            Console.WriteLine("[Player Object] Object has entered collision zone.");
        }

        [UnmanagedCallersOnly]
        public static void OnCollisionStay(IntPtr instancePtr, uint gameObjectId, uint collidingGameObjectId)
        {
            Console.WriteLine("[Player Object] Object is still in collision zone.");
        }

        [UnmanagedCallersOnly]
        public static void OnCollisionExit(IntPtr instancePtr, uint gameObjectId, uint collidingGameObjectId)
        {
            Console.WriteLine("[Player Object] Object has exited collision zone.");
        }

        [UnmanagedCallersOnly]
        public static void Update(IntPtr instancePtr, float deltaTime)
        {
            if (instancePtr == IntPtr.Zero) return;

            var instance = GameObject.GetFromPtr<GameEnemy>(instancePtr);
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
