using GameScriptLibraryDLL.Components;
using GameScriptLibraryDLL.GameObjects;
using GlmSharp;
using System.Numerics;
using System.Runtime.InteropServices;
using static GameScriptLibraryDLL.GameObjects.GameObjectVariableDLL;

namespace GameScriptLibraryDLL.GameObjects
{
    public static class PlayerScript
    {
        [UnmanagedCallersOnly] public static IntPtr Create()=> GCHandle.ToIntPtr(GCHandle.Alloc(new Player()));
        [UnmanagedCallersOnly] public static void StartUp(IntPtr instancePtr, uint id, uint parent) => GameObject.GetFromPtr<Player>(instancePtr).StartUp(instancePtr, id, parent);
        [UnmanagedCallersOnly] public static void Update(IntPtr instancePtr, float dt) => GameObject.GetFromPtr<Player>(instancePtr).Update(instancePtr, dt);
        [UnmanagedCallersOnly] public static void OnCollisionEnter(IntPtr instancePtr, uint gameObjectId, uint collidingGameObjectId) => GameObject.GetFromPtr<Player>(instancePtr).OnCollisionEnter(instancePtr, gameObjectId, collidingGameObjectId);
        [UnmanagedCallersOnly] public static void OnCollisionStay(IntPtr instancePtr, uint gameObjectId, uint collidingGameObjectId) => GameObject.GetFromPtr<Player>(instancePtr).OnCollisionEnter(instancePtr, gameObjectId, collidingGameObjectId);
        [UnmanagedCallersOnly] public static void OnCollisionExit(IntPtr instancePtr, uint gameObjectId, uint collidingGameObjectId) => GameObject.GetFromPtr<Player>(instancePtr).OnCollisionEnter(instancePtr, gameObjectId, collidingGameObjectId);
        [UnmanagedCallersOnly] public static void Destroy(IntPtr instancePtr) => GameObject.GetFromPtr<Player>(instancePtr).Destroy(instancePtr);
    }

    public unsafe class Player : GameObject, IGameObjectType
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
        public const uint PlayerSpeed = 600;
        public const float CoolDownTimer = 2.0f;
        public const uint PlayerShotMaximum = 5;
        public static GameObjectTypeEnum ObjectType => GameObjectTypeEnum.kGameObjectMegaMan;
        public uint PlayerShotCount { get; set; }
        public float CoolDownTime { get; set; } = 0;

        public override IntPtr Create()
        {
            var instance = new Player();
            GCHandle handle = GCHandle.Alloc(instance, GCHandleType.Normal);
            return GCHandle.ToIntPtr(handle);
        }

        public override void StartUp(IntPtr instancePtr, uint gameObjectId, uint parentGameObjectId)
        {
            if (instancePtr == IntPtr.Zero) return;

            var instance = GameObject.GetFromPtr<Player>(instancePtr);
            instance.ParentGameObjectId = parentGameObjectId;
            instance.GameObjectId = gameObjectId;
        }

        public override void OnCollisionEnter(IntPtr instancePtr, uint gameObjectId, uint collidingGameObjectId)
        {
            Console.WriteLine("[Player Object] Object has entered collision zone.");
        }

        public override void OnCollisionStay(IntPtr instancePtr, uint gameObjectId, uint collidingGameObjectId)
        {
            Console.WriteLine("[Player Object] Object is still in collision zone.");
        }

        public override void OnCollisionExit(IntPtr instancePtr, uint gameObjectId, uint collidingGameObjectId)
        {
            Console.WriteLine("[Player Object] Object has exited collision zone.");
        }

        public override void Update(IntPtr instancePtr, float deltaTime)
        {
            if (instancePtr == IntPtr.Zero) return;

            var instance = GameObject.GetFromPtr<Player>(instancePtr);
            //Dictionary<String, GameObjectVariable<float>> gameObjectVariableList = GameObject.GetGameObjectVariables<float>(instance.GameObjectId);
            Transform2DComponent* transform = Component.GetGameObjectComponent<Transform2DComponent>(instance.GameObjectId, ComponentTypeEnum.kTransform2DComponent);
            SpriteComponent* sprite = Component.GetGameObjectComponent<SpriteComponent>(instance.GameObjectId, ComponentTypeEnum.kSpriteComponent);
            InputComponent* input = Component.GetGameObjectComponent<InputComponent>(instance.GameObjectId, ComponentTypeEnum.kInputComponent);

            if (transform == null) return;
            bool leftPressed = input != null && (input->buttonsList[(int)GamePadButtonEnum.GLFW_GAMEPAD_BUTTON_DPAD_LEFT] == 1);
            bool rightPressed = input != null && (input->buttonsList[(int)GamePadButtonEnum.GLFW_GAMEPAD_BUTTON_DPAD_RIGHT] == 1);
            bool upPressed = input != null && (input->buttonsList[(int)GamePadButtonEnum.GLFW_GAMEPAD_BUTTON_DPAD_UP] == 1);
            bool downPressed = input != null && (input->buttonsList[(int)GamePadButtonEnum.GLFW_GAMEPAD_BUTTON_DPAD_DOWN] == 1);
            bool shootPressed = input != null && (input->buttonsList[(int)GamePadButtonEnum.GLFW_GAMEPAD_BUTTON_SQUARE] == 1);

            if (leftPressed)
            {
                sprite->FlipSprite = new ivec2(1, sprite->FlipSprite.y);
                transform->Position = new(transform->Position.x - PlayerSpeed * deltaTime, transform->Position.y);
            }
            else if (rightPressed)
            {
                sprite->FlipSprite = new ivec2(0, sprite->FlipSprite.y);
                transform->Position = new(transform->Position.x + PlayerSpeed * deltaTime, transform->Position.y);
            }

            if (upPressed)
            {
                sprite->FlipSprite = new ivec2(1, sprite->FlipSprite.y);
                transform->Position = new(transform->Position.x, transform->Position.y + PlayerSpeed * deltaTime);
            }
            else if (downPressed)
            {
                sprite->FlipSprite = new ivec2(0, sprite->FlipSprite.y);
                transform->Position = new(transform->Position.x, transform->Position.y - PlayerSpeed * deltaTime);
            }

            if (shootPressed &&
                PlayerShotMaximum > instance.PlayerShotCount &&
                (instance.CoolDownTime += deltaTime) > CoolDownTimer)

            {
                GameObject.CreateGameObject(GameObjectTypeEnum.kGameObjectMegaManShot, transform->Position, instance.GameObjectId);
                instance.CoolDownTime = 0.0f;
            }
            transform->Dirty = true;
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
