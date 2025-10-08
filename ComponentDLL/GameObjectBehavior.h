#pragma once
#include "pch.h"
struct GameObjectBehavior
{
    void* (*CreateObject)();
    void  (*KeyBoardInput)(GameObjectID gameObjectId, const float& deltaTime, const KeyState* keyBoardStateArray) = nullptr;
    void  (*ControllerInput)(GameObjectID gameObjectId, const float& deltaTime, const GLFWgamepadstate& controlelrState) = nullptr;
    void  (*Update)(GameObjectID gameObjectId, const float& deltaTime) = nullptr;
    void  (*Destroy)() = nullptr;
};
