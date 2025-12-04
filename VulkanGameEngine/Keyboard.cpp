#include <stdio.h>
#include <ctype.h>
#include "Keyboard.h"

#ifndef __ANDROID__
Keyboard keyboard = Keyboard();

Keyboard::Keyboard()
{
}

Keyboard::~Keyboard()
{
}

void Keyboard::KeyboardKeyPressed(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    switch (action)
    {
        case GLFW_PRESS: keyboard.KeyPressed[key] = KS_PRESSED; break;
        case GLFW_REPEAT: keyboard.KeyPressed[key] = KS_HELD; break;
        case GLFW_RELEASE: keyboard.KeyPressed[key] = KS_RELEASED; break;
    }
}
#endif