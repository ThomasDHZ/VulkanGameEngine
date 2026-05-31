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
    void Rotate(float x, float y);
    void Rotate(const vec2& degrees);
    void SetPosition(float x, float y);
    void SetPosition(const vec2& pos);
};

inline void Transform2DComponent::Move(float x, float y)
{
    GameObjectPosition.x += x;
    GameObjectPosition.y += y;
    Dirty = true;
}

inline void Transform2DComponent::Move(const vec2& delta)
{
    GameObjectPosition += delta;
    Dirty = true;
}

inline void Transform2DComponent::Rotate(float x, float y)
{
    GameObjectRotation.x += x;
    Dirty = true;
}

inline void Transform2DComponent::Rotate(const vec2& degrees)
{
    GameObjectRotation += degrees;
    Dirty = true;
}

inline void Transform2DComponent::SetPosition(float x, float y)
{
    GameObjectPosition = vec2(x, y);
    Dirty = true;
}

inline void Transform2DComponent::SetPosition(const vec2& pos)
{
    GameObjectPosition = pos;
    Dirty = true;
}

struct Transform3DComponent
{
    vec3 GameObjectPosition = vec3(0.0f);
    vec3 GameObjectRotation = vec3(0.0f);
    vec3 GameObjectScale = vec3(1.0f);
    bool Dirty = true;
};