#pragma once
#include "pch.h"
#include "DLL.h"
#include "GameObject.h"

struct MegaManShot
{
	//static constexpr GameObjectTypeEnum GameObjectType = GameObjectTypeEnum::kGameObjectMegaManShot;
	//static constexpr uint64 GameObjectComponentMask = kTransform2DComponent | kSpriteComponent;
	static constexpr uint MegaManShotDistance = 1000;
	uint MegaManShotDistanceTraveled = 0;
};

DLL_EXPORT void MegaManShot_CreateObject(const String& name, VkGuid vramId, vec2 objectPosition, uint parentGameObjectId);
DLL_EXPORT void MegaManShot_Behaviors(GameObjectBehavior& componentBehavior);
DLL_EXPORT void MegaManShot_Update(uint gameObjectId, const float& deltaTime);
DLL_EXPORT void MegaManShot_Destroy(uint gameObjectId);