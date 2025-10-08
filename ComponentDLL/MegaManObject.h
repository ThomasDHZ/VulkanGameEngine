#pragma once
#include "pch.h"
#include <glfw/include/GLFW/glfw3.h>
#include "GameObject.h"

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

struct MegaManObject
{
    const uint MaxShotCount = 3;
    const float MaxShotCoolDownTime = 1.0f;

};

DLL_EXPORT void MegaMan_Behaviors(GameObjectBehavior& componentBehavior);
DLL_EXPORT void MegaMan_KeyBoardInput(GameObjectID gameObjectId, const float& deltaTime, const KeyState* keyBoardStateArray);
DLL_EXPORT void MegaMan_ControllerInput(GameObjectID gameObjectId, const float& deltaTime, const GLFWgamepadstate& controlelrState);

