#pragma once
#include "DLL.h"
#include "Typedef.h"
#include "VkGuid.h"
#include "ECSid.h"
#include "Material.h"
#include "Transform2DComponent.h"
#include "VRAM.h"

struct Sprite
{
    enum SpriteAnimationEnum
    {
        kStanding,
        kWalking
    };

    GameObjectID GameObjectId;
    uint SpriteID = 0;
    uint CurrentAnimationID = 0;
    uint CurrentFrame = 0;
    VkGuid SpriteVramId;
    float CurrentFrameTime = 0.0f;
    bool SpriteAlive = true;
    ivec2 FlipSprite = ivec2(0);
    vec2 LastSpritePosition = vec2(0.0f);
    vec2 LastSpriteRotation = vec2(0.0f);
    vec2 LastSpriteScale = vec2(1.0f);
    vec2 SpritePosition = vec2(0.0f);
    vec2 SpriteRotation = vec2(0.0f);
    vec2 SpriteScale = vec2(1.0f);
};

struct SpriteInstanceStruct
{
    vec2  SpritePosition;
    vec4  UVOffset;
    vec2  SpriteSize;
    ivec2 FlipSprite;
    vec4  Color;
    mat4  InstanceTransform;
    uint  MaterialID;

    SpriteInstanceStruct()
    {
        SpritePosition = vec2(0.0f);
        UVOffset = vec4(0.0f);
        SpriteSize = vec2(0.0f);
        FlipSprite = ivec2(0);
        Color = vec4(0.0f);
        MaterialID = 0;
        InstanceTransform = mat4(1.0f);
    }

    SpriteInstanceStruct(vec2 spritePosition, vec4 uv, vec2 spriteSize, ivec2 flipSprite, vec4 color, uint materialID, mat4 instanceTransform, uint spriteLayer)
    {
        SpritePosition = spritePosition;
        UVOffset = uv;
        SpriteSize = spriteSize;
        FlipSprite = flipSprite;
        Color = color;
        MaterialID = materialID;
        InstanceTransform = instanceTransform;
    }
};

struct SpriteInstanceVertex2D
{
    vec2 SpritePosition;
    vec4 UVOffset;
    vec2 SpriteSize;
    ivec2 FlipSprite;
    vec4 Color;
    mat4 InstanceTransform;
    uint MaterialID;

    SpriteInstanceVertex2D()
    {
        SpritePosition = vec2(0.0f);
        UVOffset = vec4(0.0f);
        SpriteSize = vec2(0.0f);
        FlipSprite = ivec2(0);
        Color = vec4(0.0f);
        MaterialID = 0;
        InstanceTransform = mat4(1.0f);
    }

    SpriteInstanceVertex2D(vec2 spritePosition, vec4 uv, vec2 spriteSize, ivec2 flipSprite, vec4 color, uint materialID, mat4 instanceTransform, uint spriteLayer)
    {
        SpritePosition = spritePosition;
        UVOffset = uv;
        SpriteSize = spriteSize;
        FlipSprite = flipSprite;
        Color = color;
        MaterialID = materialID;
        InstanceTransform = instanceTransform;
    }
};

#ifdef __cplusplus
extern "C" {
#endif
    DLL_EXPORT void Sprite_UpdateBatchSprites(SpriteInstanceStruct* spriteInstanceList, Sprite* spriteList, const Transform2DComponent* transform2DList, const SpriteVram* vramList, const Animation2D* animationList, const AnimationFrames* frameList, const Material* materialList, size_t spriteCount, float deltaTime);
    DLL_EXPORT void Sprite_SetSpriteAnimation(Sprite& sprite, Sprite::SpriteAnimationEnum spriteAnimation);
#ifdef __cplusplus
}
#endif
    DLL_EXPORT SpriteInstanceStruct Sprite_UpdateSprites(const Transform2DComponent& transform2D, const SpriteVram& vram, const Animation2D& animation, const Material& material, const ivec2& currentFrame, Sprite& sprite, size_t frameCount, float deltaTime);