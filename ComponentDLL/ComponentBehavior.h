#pragma once
#include <Sprite.h>
#include <Transform2DComponent.h>
enum ObjectEnum
{
    kMegaMan,
    kMegaManShot
};

struct ComponentBehavior
{
    void* (*CreateObject)();
    void  (*KeyBoardInput)(const float& deltaTime, const KeyState* keyBoardStateArray, Sprite& sprite, Transform2DComponent& transform) = nullptr;
    void  (*ControllerInput)(const float& deltaTime, const GLFWgamepadstate& controlelrState, Sprite& sprite, Transform2DComponent& transform) = nullptr;
    void  (*Movement)(const float& deltaTime, Transform2DComponent& transform2D, bool direction) = nullptr;
    void  (*Destroy)() = nullptr;
};
