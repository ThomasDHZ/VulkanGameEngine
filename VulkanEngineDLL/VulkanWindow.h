#pragma once
#include <vulkan/vulkan_core.h>
#include <stdbool.h>
#include "Macro.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_keycode.h>
#include <glfw/include/GLFW/glfw3.h>

#ifdef __cplusplus
extern "C" {
#endif
#include "DLL.h"

#define MAXMOUSEKEY 3
#define MAXKEYBOARDKEY 350

    typedef enum Window_Type
    {
        SDL = 0,
        GLFW = 1
    } Window_Type;

    typedef enum mouseButtons
    {
        MB_Left = 1,
        MB_Middle = 2,
        MB_Right = 3
    }MouseButtons;

    typedef enum mouseButtonEventState
    {
        MS_RELEASED,
        MS_PRESSED,
        MS_HELD
    }MouseButtonEventState;

    typedef struct mouseState
    {
        int X;
        int Y;
        int WheelOffset;
        MouseButtonEventState MouseButtonState[MAXMOUSEKEY];
    }MouseState;

    typedef enum KeyboardKeyCode {
        KEY_UNKNOWN = -1,
        KEY_A = 0, KEY_B, KEY_C, KEY_D, KEY_E, KEY_F, KEY_G, KEY_H, KEY_I,
        KEY_J, KEY_K, KEY_L, KEY_M, KEY_N, KEY_O, KEY_P, KEY_Q, KEY_R,
        KEY_S, KEY_T, KEY_U, KEY_V, KEY_W, KEY_X, KEY_Y, KEY_Z,
        KEY_0, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9,
        KEY_SPACE,
        KEY_ENTER,
        KEY_ESCAPE,
        KEY_TAB,
        KEY_BACKSPACE,
        KEY_DELETE,
        KEY_LEFT, KEY_RIGHT, KEY_UP, KEY_DOWN,
        KEY_SHIFT,
        KEY_CTRL,
        KEY_ALT,
        KEY_CAPS_LOCK,
        KEY_NUM_LOCK,
        KEY_SCROLL_LOCK,
        KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6,
        KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F11, KEY_F12,
        KEY_HOME,
        KEY_END,
        KEY_PAGE_UP,
        KEY_PAGE_DOWN,
        KEY_PRINT_SCREEN,
        KEY_INSERT,
        KEY_MENU,
        KEY_LAST
    } KeyboardKeyCode;

    typedef enum inputKey
    {
        INPUTKEY_UNKNOWN = -1,
        INPUTKEY_RETURN = '\r',
        INPUTKEY_ESCAPE = '\x1B',
        INPUTKEY_BACKSPACE = '\b',
        INPUTKEY_TAB = '\t',
        INPUTKEY_SPACE = ' ',
        INPUTKEY_EXCLAIM = '!',
        INPUTKEY_QUOTEDBL = '"',
        INPUTKEY_HASH = '#',
        INPUTKEY_PERCENT = '%',
        INPUTKEY_DOLLAR = '$',
        INPUTKEY_AMPERSAND = '&',
        INPUTKEY_QUOTE = '\'',
        INPUTKEY_LEFTPAREN = '(',
        INPUTKEY_RIGHTPAREN = ')',
        INPUTKEY_ASTERISK = '*',
        INPUTKEY_PLUS = '+',
        INPUTKEY_COMMA = ',',
        INPUTKEY_MINUS = '-',
        INPUTKEY_PERIOD = '.',
        INPUTKEY_SLASH = '/',
        INPUTKEY_0 = '0',
        INPUTKEY_1 = '1',
        INPUTKEY_2 = '2',
        INPUTKEY_3 = '3',
        INPUTKEY_4 = '4',
        INPUTKEY_5 = '5',
        INPUTKEY_6 = '6',
        INPUTKEY_7 = '7',
        INPUTKEY_8 = '8',
        INPUTKEY_9 = '9',
        INPUTKEY_COLON = ':',
        INPUTKEY_SEMICOLON = ';',
        INPUTKEY_LESS = '<',
        INPUTKEY_EQUALS = '=',
        INPUTKEY_GREATER = '>',
        INPUTKEY_QUESTION = '?',
        INPUTKEY_AT = '@',
        INPUTKEY_LEFTBRACKET = '[',
        INPUTKEY_BACKSLASH = '\\',
        INPUTKEY_RIGHTBRACKET = ']',
        INPUTKEY_CARET = '^',
        INPUTKEY_UNDERSCORE = '_',
        INPUTKEY_BACKQUOTE = '`',
        INPUTKEY_A = 'a',
        INPUTKEY_B = 'b',
        INPUTKEY_C = 'c',
        INPUTKEY_D = 'd',
        INPUTKEY_E = 'e',
        INPUTKEY_F = 'f',
        INPUTKEY_G = 'g',
        INPUTKEY_H = 'h',
        INPUTKEY_I = 'i',
        INPUTKEY_J = 'j',
        INPUTKEY_K = 'k',
        INPUTKEY_L = 'l',
        INPUTKEY_M = 'm',
        INPUTKEY_N = 'n',
        INPUTKEY_O = 'o',
        INPUTKEY_P = 'p',
        INPUTKEY_Q = 'q',
        INPUTKEY_R = 'r',
        INPUTKEY_S = 's',
        INPUTKEY_T = 't',
        INPUTKEY_U = 'u',
        INPUTKEY_V = 'v',
        INPUTKEY_W = 'w',
        INPUTKEY_X = 'x',
        INPUTKEY_Y = 'y',
        INPUTKEY_Z = 'z',
        INPUTKEY_INSERT = 260,
        INPUTKEY_DELETE = 261,
        INPUTKEY_RIGHT = 262,
        INPUTKEY_LEFT = 263,
        INPUTKEY_DOWN = 264,
        INPUTKEY_UP = 265,
        INPUTKEY_PAGEUP = 266,
        INPUTKEY_PAGEDOWN = 267,
        INPUTKEY_HOME = 268,
        INPUTKEY_END = 269,
        INPUTKEY_CAPSLOCK = 280,
        INPUTKEY_SCROLLLOCK = 281,
        INPUTKEY_NUMLOCKCLEAR = 282,
        INPUTKEY_PRINTSCREEN = 283,
        INPUTKEY_PAUSE = 284,
        INPUTKEY_F1 = 290,
        INPUTKEY_F2 = 291,
        INPUTKEY_F3 = 292,
        INPUTKEY_F4 = 293,
        INPUTKEY_F5 = 294,
        INPUTKEY_F6 = 295,
        INPUTKEY_F7 = 296,
        INPUTKEY_F8 = 297,
        INPUTKEY_F9 = 298,
        INPUTKEY_F10 = 299,
        INPUTKEY_F11 = 300,
        INPUTKEY_F12 = 301,
        INPUTKEY_F13 = 302,
        INPUTKEY_F14 = 303,
        INPUTKEY_F15 = 304,
        INPUTKEY_F16 = 305,
        INPUTKEY_F17 = 306,
        INPUTKEY_F18 = 307,
        INPUTKEY_F19 = 308,
        INPUTKEY_F20 = 309,
        INPUTKEY_F21 = 310,
        INPUTKEY_F22 = 311,
        INPUTKEY_F23 = 312,
        INPUTKEY_F24 = 313,
        INPUTKEY_KP_1 = 320,
        INPUTKEY_KP_2 = 321,
        INPUTKEY_KP_3 = 322,
        INPUTKEY_KP_4 = 323,
        INPUTKEY_KP_5 = 324,
        INPUTKEY_KP_6 = 325,
        INPUTKEY_KP_7 = 326,
        INPUTKEY_KP_8 = 327,
        INPUTKEY_KP_9 = 328,
        INPUTKEY_KP_0 = 329,
        INPUTKEY_KP_PERIOD = 330,
        INPUTKEY_KP_DIVIDE = 331,
        INPUTKEY_KP_MULTIPLY = 332,
        INPUTKEY_KP_MINUS = 333,
        INPUTKEY_KP_PLUS = 334,
        INPUTKEY_KP_ENTER = 335,
        INPUTKEY_KP_EQUALS = 336,
        INPUTKEY_LSHIFT = 340,
        INPUTKEY_LCTRL = 341,
        INPUTKEY_LALT = 342,
        INPUTKEY_RSHIFT = 344,
        INPUTKEY_RCTRL = 345,
        INPUTKEY_RALT = 346,
        INPUTKEY_MENU = 348
    }InputKey;

    typedef enum keyState
    {
        KS_RELEASED,
        KS_PRESSED,
        KS_HELD
    }KeyState;

    typedef struct keyboardState
    {
        KeyState KeyPressed[MAXKEYBOARDKEY];
    }KeyboardState;

    typedef struct VulkanWindow
    {
        Window_Type WindowType;
        void* WindowHandle;
        uint32_t    Width;
        uint32_t    Height;
        bool        FrameBufferResized;
        bool        ShouldClose;
        MouseState  mouse;
        KeyboardState keyboard;

        void (*CreateGraphicsWindow)(struct VulkanWindow* self, const char* WindowName, uint32_t width, uint32_t height);
        void (*PollEventHandler)(struct VulkanWindow* self);
        void (*SwapBuffer)(struct VulkanWindow* self);
        const char** (*GetInstanceExtensions)(struct VulkanWindow* self, uint32_t* outExtensionCount, bool enableValidationLayers);
        void (*CreateSurface)(struct VulkanWindow* self, VkInstance* instance, VkSurfaceKHR* surface);
        void (*GetFrameBufferSize)(struct VulkanWindow* self, int* width, int* height);
        void (*DestroyWindow)(struct VulkanWindow* self);
        bool (*WindowShouldClose)(struct VulkanWindow* self);
    } VulkanWindow;
    DLL_EXPORT extern VulkanWindow* vulkanWindow;

    DLL_EXPORT KeyboardKeyCode ConvertGLFWKeyToUnified(int glfwKey);
    DLL_EXPORT KeyboardKeyCode ConvertSDLKeyToUnified(SDL_Keycode sdlKey);
    DLL_EXPORT VulkanWindow* Window_CreateWindow(Window_Type windowType, const char* WindowName, uint32_t width, uint32_t height);
#ifdef __cplusplus
}
#endif