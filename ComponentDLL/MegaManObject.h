#pragma once
#include "pch.h"
#include <glfw/include/GLFW/glfw3.h>

enum MegaManAnimationEnum
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

struct MegaManStruct
{

};

DLL_EXPORT void MegaMan_Behaviors(ComponentBehavior& componentBehavior);
DLL_EXPORT void MegaMan_KeyBoardInput(const float& deltaTime, const KeyState* keyBoardStateArray, Sprite& sprite, Transform2DComponent& transform);
DLL_EXPORT void MegaMan_ControllerInput(const float& deltaTime, const GLFWgamepadstate& controlelrState, Sprite& sprite, Transform2DComponent& transform);

