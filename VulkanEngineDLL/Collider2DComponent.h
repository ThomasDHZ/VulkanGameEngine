#pragma once
#include <Platform.h>

struct Collider2DComponent
{
    ivec2 Size = ivec2(32, 32);
    ivec2 Offset = ivec2(0, 0);
    bool Enabled = true;
    bool IsTrigger = false;
};

