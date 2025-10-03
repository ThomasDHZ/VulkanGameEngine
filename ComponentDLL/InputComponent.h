#pragma once
#include "ECSid.h"

struct InputComponent
{
	GameObjectID GameObjectId;

	InputComponent()
	{
		GameObjectId.id = 0;
	}

	InputComponent(GameObjectID gameObjectId)
	{
		GameObjectId = gameObjectId;
	}

	~InputComponent()
	{

	}
};

