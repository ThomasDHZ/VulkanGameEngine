using GlmSharp;
using System.Numerics;
using System.Runtime.InteropServices;

namespace GameScriptLibraryDLL
{
    public unsafe class Player
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
        }

        [UnmanagedCallersOnly]
        public static void Update(IntPtr instancePtr, float deltaTime)
        {
            if (instancePtr == IntPtr.Zero) return;

            var instance = (Player)GCHandle.FromIntPtr(instancePtr).Target;
            InputComponent* input = Component.Get<InputComponent>(instance.GameObjectId, ComponentTypeEnum.kInputComponent);
            SpriteComponent* sprite = Component.Get<SpriteComponent>(instance.GameObjectId, ComponentTypeEnum.kSpriteComponent);
            Transform2DComponent* transform = Component.Get<Transform2DComponent>(instance.GameObjectId, ComponentTypeEnum.kTransform2DComponent);

            bool leftPressed =  input->KeyBoardState[(int)KeyboardKeyCode.KEY_A] == KeyState.KS_PRESSED ||
                                input->KeyBoardState[(int)KeyboardKeyCode.KEY_A] == KeyState.KS_HELD;
            bool rightPressed = input->KeyBoardState[(int)KeyboardKeyCode.KEY_D] == KeyState.KS_PRESSED ||
                                input->KeyBoardState[(int)KeyboardKeyCode.KEY_D] == KeyState.KS_HELD;
            bool upPressed =    input->KeyBoardState[(int)KeyboardKeyCode.KEY_W] == KeyState.KS_PRESSED ||
                                input->KeyBoardState[(int)KeyboardKeyCode.KEY_W] == KeyState.KS_HELD;
            bool downPressed =  input->KeyBoardState[(int)KeyboardKeyCode.KEY_S] == KeyState.KS_PRESSED ||
                                input->KeyBoardState[(int)KeyboardKeyCode.KEY_S] == KeyState.KS_HELD;
            bool shootPressed = input->KeyBoardState[(int)KeyboardKeyCode.KEY_E] == KeyState.KS_PRESSED ||
                                input->KeyBoardState[(int)KeyboardKeyCode.KEY_E] == KeyState.KS_HELD;

            if (leftPressed)
            {
                sprite->FlipSprite = new ivec2(1, sprite->FlipSprite.y);
                transform->Position = new(transform->Position.x - 200.0f * deltaTime, transform->Position.y);
            }
            else if (rightPressed)
            {
                sprite->FlipSprite = new ivec2(0, sprite->FlipSprite.y);
                transform->Position = new(transform->Position.x + 200.0f * deltaTime, transform->Position.y);
            }

            if (upPressed) transform->Position = new(transform->Position.x, transform->Position.y - 200.0f * deltaTime);
            if (downPressed) transform->Position = new(transform->Position.x, transform->Position.y - 200.0f * deltaTime);
            transform->Dirty = true;
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