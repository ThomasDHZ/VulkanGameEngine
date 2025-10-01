#pragma once
#include "pch.h"

class MegaManShot
{
};

DLL_EXPORT void MegaManShot_Behaviors(ComponentBehavior& componentBehavior);
DLL_EXPORT void MegaManShot_Movement(const float& deltaTime, Transform2DComponent& transform2D, bool direction);
DLL_EXPORT void MegaManShot_Destroy();