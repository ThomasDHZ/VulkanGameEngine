#pragma once
#include "memory"
#include "Typedef.h"

struct Transform2DComponent
{
    vec2 GameObjectPosition = vec2(0.0f);
    vec2 GameObjectRotation = vec2(0.0f);
    vec2 GameObjectScale = vec2(1.0f);
    bool Dirty = true;

    void Move(float x, float y);
    void Move(const vec2& delta);
    void SetPosition(float x, float y);
    void SetPosition(const vec2& pos);
};

struct Transform3DComponent
{
    vec3 GameObjectPosition = vec3(0.0f);
    vec3 GameObjectRotation = vec3(0.0f);
    vec3 GameObjectScale = vec3(1.0f);
    bool Dirty = true;
};