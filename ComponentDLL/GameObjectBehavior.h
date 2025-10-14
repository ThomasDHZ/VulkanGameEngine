#pragma once
#include "pch.h"
struct GameObjectBehavior
{
    void (*CreateObject)(const String& name, VkGuid vramId, vec2 objectPosition, uint parentGameObjectId);
    void (*KeyBoardInput)(uint gameObjectId, const float& deltaTime, const KeyState* keyBoardStateArray) = nullptr;
    void (*ControllerInput)(uint gameObjectId, const float& deltaTime, const GLFWgamepadstate& controlelrState) = nullptr;
    void (*Update)(uint gameObjectId, const float& deltaTime) = nullptr;
    void (*Destroy)(uint gameObjectId) = nullptr;
};
