#include <stdio.h>
#include <ctype.h>
#include "Keyboard.h"
//#include <SDL2/SDL_events.h>
//
////static void UpdateKeyState(VulkanWindow* self, const SDL_Event* event, KeyState keyState)
////{
////    KeyCode unifiedKey = ConvertSDLKeyToUnified(event->key.keysym.sym);
////    switch (event->type) 
////    {
////        case SDL_KEYDOWN:
////        {
////            if (self->keyboard.KeyPressed[unifiedKey] == KS_PRESSED)
////            {
////                self->keyboard.KeyPressed[unifiedKey] = KS_HELD;
////            }
////            else
////            {
////                self->keyboard.KeyPressed[unifiedKey] = KS_PRESSED;
////            }
////            break;
////        }
////        case SDL_KEYUP:
////        {
////            self->keyboard.KeyPressed[unifiedKey] = KS_RELEASED;
////            break;
////        }
////    }
////}
////
////void GameEngine_SDL_KeyboardKeyPressed(VulkanWindow* self, const SDL_Event* event)
////{
////    UpdateKeyState(self, event, KS_PRESSED);
////}
////
////void GameEngine_SDL_KeyboardKeyReleased(VulkanWindow* self, const SDL_Event* event)
////{
////    UpdateKeyState(self, event, KS_RELEASED);
////}
//
//void GameEngine_GLFW_KeyboardKeyPressed(GLFWwindow* window, int key, int scancode, int action, int mods)
//{
//    KeyboardKeyCode unifiedKey = ConvertGLFWKeyToUnified(key);
//    switch (action)
//    {
//    case GLFW_PRESS: vulkanWindow->keyboard.KeyPressed[unifiedKey] = KS_PRESSED; break;
//    case GLFW_REPEAT: vulkanWindow->keyboard.KeyPressed[unifiedKey] = KS_HELD; break;
//    case GLFW_RELEASE: vulkanWindow->keyboard.KeyPressed[unifiedKey] = KS_RELEASED; break;
//    }
//}
//
//KeyboardKeyCode ConvertGLFWKeyToUnified(int glfwKey)
//{
//    switch (glfwKey)
//    {
//    case GLFW_KEY_A: return KEY_A;
//    case GLFW_KEY_B: return KEY_B;
//    case GLFW_KEY_C: return KEY_C;
//    case GLFW_KEY_D: return KEY_D;
//    case GLFW_KEY_E: return KEY_E;
//    case GLFW_KEY_F: return KEY_F;
//    case GLFW_KEY_G: return KEY_G;
//    case GLFW_KEY_H: return KEY_H;
//    case GLFW_KEY_I: return KEY_I;
//    case GLFW_KEY_J: return KEY_J;
//    case GLFW_KEY_K: return KEY_K;
//    case GLFW_KEY_L: return KEY_L;
//    case GLFW_KEY_M: return KEY_M;
//    case GLFW_KEY_N: return KEY_N;
//    case GLFW_KEY_O: return KEY_O;
//    case GLFW_KEY_P: return KEY_P;
//    case GLFW_KEY_Q: return KEY_Q;
//    case GLFW_KEY_R: return KEY_R;
//    case GLFW_KEY_S: return KEY_S;
//    case GLFW_KEY_T: return KEY_T;
//    case GLFW_KEY_U: return KEY_U;
//    case GLFW_KEY_V: return KEY_V;
//    case GLFW_KEY_W: return KEY_W;
//    case GLFW_KEY_X: return KEY_X;
//    case GLFW_KEY_Y: return KEY_Y;
//    case GLFW_KEY_Z: return KEY_Z;
//    case GLFW_KEY_0: return KEY_0;
//    case GLFW_KEY_1: return KEY_1;
//    case GLFW_KEY_2: return KEY_2;
//    case GLFW_KEY_3: return KEY_3;
//    case GLFW_KEY_4: return KEY_4;
//    case GLFW_KEY_5: return KEY_5;
//    case GLFW_KEY_6: return KEY_6;
//    case GLFW_KEY_7: return KEY_7;
//    case GLFW_KEY_8: return KEY_8;
//    case GLFW_KEY_9: return KEY_9;
//    case GLFW_KEY_SPACE: return KEY_SPACE;
//    case GLFW_KEY_ENTER: return KEY_ENTER;
//    case GLFW_KEY_ESCAPE: return KEY_ESCAPE;
//    case GLFW_KEY_TAB: return KEY_TAB;
//    case GLFW_KEY_BACKSPACE: return KEY_BACKSPACE;
//    case GLFW_KEY_DELETE: return KEY_DELETE;
//    case GLFW_KEY_LEFT: return KEY_LEFT;
//    case GLFW_KEY_RIGHT: return KEY_RIGHT;
//    case GLFW_KEY_UP: return KEY_UP;
//    case GLFW_KEY_DOWN: return KEY_DOWN;
//    case GLFW_KEY_LEFT_SHIFT: return KEY_SHIFT;
//    case GLFW_KEY_RIGHT_SHIFT: return KEY_SHIFT;
//    case GLFW_KEY_LEFT_CONTROL: return KEY_CTRL;
//    case GLFW_KEY_RIGHT_CONTROL: return KEY_CTRL;
//    case GLFW_KEY_LEFT_ALT: return KEY_ALT;
//    case GLFW_KEY_RIGHT_ALT: return KEY_ALT;
//    case GLFW_KEY_CAPS_LOCK: return KEY_CAPS_LOCK;
//    case GLFW_KEY_NUM_LOCK: return KEY_NUM_LOCK;
//    case GLFW_KEY_SCROLL_LOCK: return KEY_SCROLL_LOCK;
//    case GLFW_KEY_F1: return KEY_F1;
//    case GLFW_KEY_F2: return KEY_F2;
//    case GLFW_KEY_F3: return KEY_F3;
//    case GLFW_KEY_F4: return KEY_F4;
//    case GLFW_KEY_F5: return KEY_F5;
//    case GLFW_KEY_F6: return KEY_F6;
//    case GLFW_KEY_F7: return KEY_F7;
//    case GLFW_KEY_F8: return KEY_F8;
//    case GLFW_KEY_F9: return KEY_F9;
//    case GLFW_KEY_F10: return KEY_F10;
//    case GLFW_KEY_F11: return KEY_F11;
//    case GLFW_KEY_F12: return KEY_F12;
//    case GLFW_KEY_HOME: return KEY_HOME;
//    case GLFW_KEY_END: return KEY_END;
//    case GLFW_KEY_PAGE_UP: return KEY_PAGE_UP;
//    case GLFW_KEY_PAGE_DOWN: return KEY_PAGE_DOWN;
//    case GLFW_KEY_PRINT_SCREEN: return KEY_PRINT_SCREEN;
//    case GLFW_KEY_INSERT: return KEY_INSERT;
//    case GLFW_KEY_MENU: return KEY_MENU;
//    default: return KEY_UNKNOWN;
//    }
//}
//
////KeyCode ConvertSDLKeyToUnified(SDL_Keycode sdlKey)
////{
////    switch (sdlKey)
////    {
////    case SDLK_a: return KEY_A;
////    case SDLK_b: return KEY_B;
////    case SDLK_c: return KEY_C;
////    case SDLK_d: return KEY_D;
////    case SDLK_e: return KEY_E;
////    case SDLK_f: return KEY_F;
////    case SDLK_g: return KEY_G;
////    case SDLK_h: return KEY_H;
////    case SDLK_i: return KEY_I;
////    case SDLK_j: return KEY_J;
////    case SDLK_k: return KEY_K;
////    case SDLK_l: return KEY_L;
////    case SDLK_m: return KEY_M;
////    case SDLK_n: return KEY_N;
////    case SDLK_o: return KEY_O;
////    case SDLK_p: return KEY_P;
////    case SDLK_q: return KEY_Q;
////    case SDLK_r: return KEY_R;
////    case SDLK_s: return KEY_S;
////    case SDLK_t: return KEY_T;
////    case SDLK_u: return KEY_U;
////    case SDLK_v: return KEY_V;
////    case SDLK_w: return KEY_W;
////    case SDLK_x: return KEY_X;
////    case SDLK_y: return KEY_Y;
////    case SDLK_z: return KEY_Z;
////    case SDLK_0: return KEY_0;
////    case SDLK_1: return KEY_1;
////    case SDLK_2: return KEY_2;
////    case SDLK_3: return KEY_3;
////    case SDLK_4: return KEY_4;
////    case SDLK_5: return KEY_5;
////    case SDLK_6: return KEY_6;
////    case SDLK_7: return KEY_7;
////    case SDLK_8: return KEY_8;
////    case SDLK_9: return KEY_9;
////    case SDLK_SPACE: return KEY_SPACE;
////    case SDLK_RETURN: return KEY_ENTER;
////    case SDLK_ESCAPE: return KEY_ESCAPE;
////    case SDLK_TAB: return KEY_TAB;
////    case SDLK_BACKSPACE: return KEY_BACKSPACE;
////    case SDLK_DELETE: return KEY_DELETE;
////    case SDLK_LEFT: return KEY_LEFT;
////    case SDLK_RIGHT: return KEY_RIGHT;
////    case SDLK_UP: return KEY_UP;
////    case SDLK_DOWN: return KEY_DOWN;
////    case SDLK_LSHIFT: return KEY_SHIFT;
////    case SDLK_RSHIFT: return KEY_SHIFT;
////    case SDLK_LCTRL: return KEY_CTRL;
////    case SDLK_RCTRL: return KEY_CTRL;
////    case SDLK_LALT: return KEY_ALT;
////    case SDLK_RALT: return KEY_ALT;
////    case SDLK_CAPSLOCK: return KEY_CAPS_LOCK;
////    case SDLK_NUMLOCKCLEAR: return KEY_NUM_LOCK;
////    case SDLK_F1: return KEY_F1;
////    case SDLK_F2: return KEY_F2;
////    case SDLK_F3: return KEY_F3;
////    case SDLK_F4: return KEY_F4;
////    case SDLK_F5: return KEY_F5;
////    case SDLK_F6: return KEY_F6;
////    case SDLK_F7: return KEY_F7;
////    case SDLK_F8: return KEY_F8;
////    case SDLK_F9: return KEY_F9;
////    case SDLK_F10: return KEY_F10;
////    case SDLK_F11: return KEY_F11;
////    case SDLK_F12: return KEY_F12;
////    case SDLK_HOME: return KEY_HOME;
////    case SDLK_END: return KEY_END;
////    case SDLK_PAGEUP: return KEY_PAGE_UP;
////    case SDLK_PAGEDOWN: return KEY_PAGE_DOWN;
////    case SDLK_PRINTSCREEN: return KEY_PRINT_SCREEN;
////    case SDLK_INSERT: return KEY_INSERT;
////    case SDLK_MENU: return KEY_MENU;
////    default: return KEY_UNKNOWN;
////    }
////}