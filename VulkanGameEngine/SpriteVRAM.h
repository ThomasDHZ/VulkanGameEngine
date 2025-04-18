#pragma once
#include "animation2D.h"

struct SpriteVRAM
{
	uint VRAMSpriteID = 0;
	uint SpritesheetID = 0;
	uint SpriteMaterialID = 0;
	uint SpriteLayer = 0;
	vec2 SpriteSize = vec2(50.0f);
	vec4 SpriteColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
	Vector<Animation2D> AnimationList;
};