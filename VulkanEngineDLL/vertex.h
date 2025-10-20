#pragma once
#include "Typedef.h"

struct Vertex2D
{
    vec2 Position = vec2(0.0f);
    vec2 UV = vec2(0.0f);

    Vertex2D()
    {
        Position = vec2(0.0f);
        UV = vec2(0.0f);
    }

    Vertex2D(vec2 position, vec2 uv)
    {
        Position = position;
        UV = uv;
    }
};
