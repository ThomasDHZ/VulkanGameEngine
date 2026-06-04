using GameScriptLibraryDLL.Components;
using GlmSharp;
using System.Numerics;
using System.Runtime.InteropServices;
using static GameScriptLibraryDLL.GameObjects.GameObjectVariableDLL;

namespace GameScriptLibraryDLL.GameObjects
{
    public unsafe class Player : GameObject
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

        public const uint PlayerShotMaximum = 5;
        public uint PlayerShotCount { get; set; }

        [UnmanagedCallersOnly]
        public static IntPtr Create()
        {
            var instance = new Player();
            GCHandle handle = GCHandle.Alloc(instance, GCHandleType.Normal);
            return GCHandle.ToIntPtr(handle);
        }

        [UnmanagedCallersOnly]
        public static void StartUp(IntPtr instancePtr, uint gameObjectId, uint parentGameObjectId)
        {
            if (instancePtr == IntPtr.Zero) return;

            var instance = GameObject.GetFromPtr<Player>(instancePtr);
            instance.ParentGameObjectId = parentGameObjectId;
            instance.GameObjectId = gameObjectId;
        }

        [UnmanagedCallersOnly]
        public static void Update(IntPtr instancePtr, float deltaTime)
        {
            if (instancePtr == IntPtr.Zero) return;

            var instance = GameObject.GetFromPtr<Player>(instancePtr);
            Dictionary<String, GameObjectVariable<float>> gameObjectVariableList = GameObject.GetGameObjectVariables<float>(instance.GameObjectId);
            Transform2DComponent* transform = Component.GetGameObjectComponent<Transform2DComponent>(instance.GameObjectId, ComponentTypeEnum.kTransform2DComponent);
            SpriteComponent* sprite = Component.GetGameObjectComponent<SpriteComponent>(instance.GameObjectId, ComponentTypeEnum.kSpriteComponent);
            InputComponent* input = Component.GetGameObjectComponent<InputComponent>(instance.GameObjectId, ComponentTypeEnum.kInputComponent);

            if (transform == null) return;
            float playerSpeed = gameObjectVariableList["PlayerSpeed"].Value;

            bool leftPressed = input != null &&
                (input->KeyBoardState[(int)KeyboardKeyCode.KEY_A] == KeyState.KS_PRESSED ||
                 input->KeyBoardState[(int)KeyboardKeyCode.KEY_A] == KeyState.KS_HELD);

            bool rightPressed = input != null &&
                (input->KeyBoardState[(int)KeyboardKeyCode.KEY_D] == KeyState.KS_PRESSED ||
                 input->KeyBoardState[(int)KeyboardKeyCode.KEY_D] == KeyState.KS_HELD);

            bool shootPressed = input != null &&
                (input->KeyBoardState[(int)KeyboardKeyCode.KEY_E] == KeyState.KS_PRESSED ||
                 input->KeyBoardState[(int)KeyboardKeyCode.KEY_E] == KeyState.KS_HELD);


            if (leftPressed)
            {
                sprite->FlipSprite = new ivec2(1, sprite->FlipSprite.y);
                transform->Position = new(transform->Position.x - playerSpeed * deltaTime, transform->Position.y);
            }
            else if (rightPressed)
            {
                sprite->FlipSprite = new ivec2(0, sprite->FlipSprite.y);
                transform->Position = new(transform->Position.x + playerSpeed * deltaTime, transform->Position.y);
            }

            if (shootPressed &&
                PlayerShotMaximum > instance.PlayerShotCount)

            {
                GameObject.CreateGameObject(GameObjectTypeEnum.kGameObjectMegaManShot, transform->Position, instance.GameObjectId);
            }

            transform->Dirty = true;

            Console.WriteLine($"[C#] Player Update - Pos: ({transform->Position.x:F1}, {transform->Position.y:F1})");
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