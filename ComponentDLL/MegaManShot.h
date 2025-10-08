#pragma once
#include "pch.h"
#include "DLL.h"
#include "GameObject.h"

DLL_EXPORT void MegaManShot_Behaviors(GameObjectBehavior& componentBehavior);
DLL_EXPORT void MegaManShot_Update(GameObjectID gameObjectId, const float& deltaTime);
DLL_EXPORT void MegaManShot_Destroy();