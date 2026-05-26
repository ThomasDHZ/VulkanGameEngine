using GlmSharp;
using System.Runtime.InteropServices;
//using VulkanGameEngineLevelEditor.GameEngine;
//using VulkanGameEngineLevelEditor.GameEngine.Components;

namespace GameScriptLibraryDLL
{
    public class Player
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

        public delegate IntPtr CreateDelegate();
        public delegate void StartUpDelegate(IntPtr instance);
        public delegate void UpdateDelegate(IntPtr instance, float deltaTime);
        public delegate void DestroyDelegate(IntPtr instance);

        public uint GameObjectId { get; private set; }

        [UnmanagedCallersOnly]
        public static IntPtr Create()
        {
            Console.WriteLine("[C#] Player_Create called from native!");
            var instance = new Player();
            GCHandle handle = GCHandle.Alloc(instance, GCHandleType.Normal);
            return GCHandle.ToIntPtr(handle);
        }

        [UnmanagedCallersOnly]
        public static void StartUp(IntPtr instancePtr, uint gameObjectId)
        {
            if (instancePtr == IntPtr.Zero) return;

            var instance = (Player)GCHandle.FromIntPtr(instancePtr).Target;
            instance.GameObjectId = gameObjectId;

            Console.WriteLine("[C#] Player.StartUp called");
        }

        [UnmanagedCallersOnly]
        public static void Update(IntPtr instancePtr, float deltaTime)
        {
            if (instancePtr == IntPtr.Zero) return;

            var instance = (Player)GCHandle.FromIntPtr(instancePtr).Target;
            ref InputComponent input = ref Component.UpdateGameObjectComponent<InputComponent>(instance.GameObjectId, ComponentTypeEnum.kInputComponent);
            ref SpriteComponent sprite = ref Component.UpdateGameObjectComponent<SpriteComponent>(instance.GameObjectId, ComponentTypeEnum.kSpriteComponent);
            ref Transform2DComponent transform = ref Component.UpdateGameObjectComponent<Transform2DComponent>(instance.GameObjectId, ComponentTypeEnum.kTransform2DComponent);

            bool leftPressed = input.KeyBoardState[(int)KeyboardKeyCode.KEY_A] == KeyState.KS_PRESSED ||
                                input.KeyBoardState[(int)KeyboardKeyCode.KEY_A] == KeyState.KS_HELD;
            bool rightPressed = input.KeyBoardState[(int)KeyboardKeyCode.KEY_D] == KeyState.KS_PRESSED ||
                                input.KeyBoardState[(int)KeyboardKeyCode.KEY_D] == KeyState.KS_HELD;
            bool upPressed = input.KeyBoardState[(int)KeyboardKeyCode.KEY_W] == KeyState.KS_PRESSED ||
                                input.KeyBoardState[(int)KeyboardKeyCode.KEY_W] == KeyState.KS_HELD;
            bool downPressed = input.KeyBoardState[(int)KeyboardKeyCode.KEY_S] == KeyState.KS_PRESSED ||
                                input.KeyBoardState[(int)KeyboardKeyCode.KEY_S] == KeyState.KS_HELD;
            bool shootPressed = input.KeyBoardState[(int)KeyboardKeyCode.KEY_E] == KeyState.KS_PRESSED ||
                                input.KeyBoardState[(int)KeyboardKeyCode.KEY_E] == KeyState.KS_HELD;
            if (leftPressed)
            {
                sprite.FlipSprite = new ivec2(0, sprite.FlipSprite.y);
                transform.Position = new(transform.Position.x - 200.0f * deltaTime, transform.Position.y);
            }
            else if (rightPressed)
            {
                sprite.FlipSprite = new ivec2(1, sprite.FlipSprite.y);
                transform.Position = new(transform.Position.x + 200.0f * deltaTime, transform.Position.y);
            }

            if (upPressed) transform.Position = new(transform.Position.x, transform.Position.y - 200.0f * deltaTime);
            if (downPressed) transform.Position = new(transform.Position.x, transform.Position.y - 200.0f * deltaTime);
            transform.Dirty = true;

        //Optional: Animation logic
        //     if (shootPressed) Sprite_SetSpriteAnimation(...);
        //    else if (leftPressed || rightPressed) Sprite_SetSpriteAnimation(...);
        //    else Sprite_SetSpriteAnimation(...);
        }

        [UnmanagedCallersOnly]
        public static void Destroy(IntPtr instance)
        {
            Console.WriteLine("[C#] Player.Destroy called");

            if (instance != IntPtr.Zero)
            {
                GCHandle handle = GCHandle.FromIntPtr(instance);
                handle.Free();                    // ← Must free!
            }
        }
    }
}