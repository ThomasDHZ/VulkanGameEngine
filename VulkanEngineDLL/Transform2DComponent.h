#pragma once
#include "memory"

struct Transform2DComponent
{
    uint GameObjectId = -1;
    vec2 GameObjectPosition = vec2(0.0f);
    vec2 GameObjectRotation = vec2(0.0f);
    vec2 GameObjectScale = vec2(1.0f);
};