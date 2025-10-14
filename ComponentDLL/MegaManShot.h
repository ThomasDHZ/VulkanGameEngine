#pragma once
#include "pch.h"
#include "DLL.h"
#include "GameObject.h"

struct MegaManShot
{
	static constexpr uint MegaManShotDistance = 1000;
	uint MegaManShotDistanceTraveled = 0;
};

DLL_EXPORT void MegaManShot_Behaviors(GameObjectBehavior& componentBehavior);
DLL_EXPORT void MegaManShot_Update(uint gameObjectId, const float& deltaTime);
DLL_EXPORT void MegaManShot_Destroy(uint gameObjectId);