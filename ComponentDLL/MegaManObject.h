#pragma once
#include "pch.h"
#include "GameObjectSystem.h"

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
    static constexpr uint MaxShotCount = 2;
    static constexpr float CoolDownTime = 0.08f;
    static constexpr vec2  ShotPostionOffset = vec2(30 * 5, 9 * 5);

    uint CurrentShotCount = 0;
    float CurrentShotTime = 0.0f;
};

DLL_EXPORT void MegaMan_Behaviors(GameObjectBehavior& componentBehavior);
DLL_EXPORT void MegaMan_Update(uint gameObjectId, const float& deltaTime);
DLL_EXPORT void MegaMan_KeyBoardInput(uint gameObjectId, const float& deltaTime, const KeyState* keyBoardStateArray);
DLL_EXPORT void MegaMan_ControllerInput(uint gameObjectId, const float& deltaTime, const GLFWgamepadstate& controlelrState);

