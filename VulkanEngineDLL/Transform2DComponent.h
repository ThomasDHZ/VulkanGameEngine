#pragma once
#include "memory"

struct Transform2DComponent
{
    vec2 GameObjectPosition = vec2(0.0f);
    vec2 GameObjectRotation = vec2(0.0f);
    vec2 GameObjectScale = vec2(1.0f);
    bool Dirty = true;
};