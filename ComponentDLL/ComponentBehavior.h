#pragma once
#include <Sprite.h>
#include <Transform2DComponent.h>
enum ObjectEnum
{
    kMegaMan
};

struct ComponentBehavior
{
    void (*KeyBoardInput)(const float& deltaTime, const KeyState* keyBoardStateArray, Sprite& sprite, Transform2DComponent& transform) = nullptr;
    void (*ControllerInput)(const float& deltaTime, const GLFWgamepadstate& controlelrState, Sprite& sprite, Transform2DComponent& transform) = nullptr;
    void (*Movement)(const float& deltaTime, Transform2DComponent& transform2D) = nullptr;
    void (*Destroy)() = nullptr;
};
