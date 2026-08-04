using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace GameScriptLibraryDLL.Components
{
    public enum GamePadButtonEnum
    {
        GLFW_GAMEPAD_BUTTON_A = 0,
        GLFW_GAMEPAD_BUTTON_B = 1,
        GLFW_GAMEPAD_BUTTON_X = 2,
        GLFW_GAMEPAD_BUTTON_Y = 3,
        GLFW_GAMEPAD_BUTTON_LEFT_BUMPER = 4,
        GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER = 5,
        GLFW_GAMEPAD_BUTTON_BACK = 6,
        GLFW_GAMEPAD_BUTTON_START = 7,
        GLFW_GAMEPAD_BUTTON_GUIDE = 8,
        GLFW_GAMEPAD_BUTTON_LEFT_THUMB = 9,
        GLFW_GAMEPAD_BUTTON_RIGHT_THUMB = 10,
        GLFW_GAMEPAD_BUTTON_DPAD_UP = 11,
        GLFW_GAMEPAD_BUTTON_DPAD_RIGHT = 12,
        GLFW_GAMEPAD_BUTTON_DPAD_DOWN = 13,
        GLFW_GAMEPAD_BUTTON_DPAD_LEFT = 14,
        GLFW_GAMEPAD_BUTTON_LAST = GLFW_GAMEPAD_BUTTON_DPAD_LEFT,
        GLFW_GAMEPAD_BUTTON_CROSS = GLFW_GAMEPAD_BUTTON_A,
        GLFW_GAMEPAD_BUTTON_CIRCLE = GLFW_GAMEPAD_BUTTON_B,
        GLFW_GAMEPAD_BUTTON_SQUARE = GLFW_GAMEPAD_BUTTON_X,
        GLFW_GAMEPAD_BUTTON_TRIANGLE = GLFW_GAMEPAD_BUTTON_Y,
    };

    public unsafe struct GamepadState
    {
        public byte* Buttons;
        public float* Axes;

        public GamepadState()
        {
   
        }
    }

    public unsafe struct InputComponent
    {
        public const uint MAXKEYBOARDKEY = 350;
        public const uint MAXMOUSEKEY = 3;

        public KeyState* KeyBoardState { get; set; }
        public float MouseX { get; set; } = 0.0f;
        public float MouseY { get; set; } = 0.0f;
        public float MouseDeltaX { get; set; } = 0.0f;
        public float MouseDeltaY { get; set; } = 0.0f;
        public bool* MouseButtons { get; set; }
        public bool HasGamepad { get; set; }
        public byte* buttonsList { get; set; }
        public float* axesList { get; set; }
        public float DeltaTime { get; set; } = 0.0f;
        public uint Sequence { get; set; } = 0;
        size_t KeyBoardButtonSize { get; set; }
        size_t MouseButtonKeySize { get; set; }
        size_t MaxButtonCount { get; set; }
        size_t MaxAxesCount { get; set; }

        public InputComponent(bool allocate = true)
        {
            MouseX = 0;
            MouseY = 0;
            MouseDeltaX = 0;
            MouseDeltaY = 0;
            HasGamepad = false;

            DeltaTime = 0;
            Sequence = 0;
        }
    }
}
