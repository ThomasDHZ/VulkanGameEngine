#include "Sprite.h"
#include "VulkanBuffer.h"

void Sprite_UpdateBatchSprites(SpriteInstanceStruct* spriteInstanceList, Sprite* spriteList, const Transform2DComponent* transform2DList, const SpriteVram* vramList, const Animation2D* animationList, const Material* materialList, size_t spriteCount, float deltaTime)
{
    Span<ivec2> frameList(animationList->FrameList, animationList->FrameList + animationList->FrameCount);
    for (size_t x = 0; x < spriteCount; x++)
    {
        glm::mat4 spriteMatrix = glm::mat4(1.0f);
        if (spriteList[x].LastSpritePosition != spriteList[x].SpritePosition) {
            spriteMatrix = glm::translate(spriteMatrix, glm::vec3(transform2DList[x].GameObjectPosition.x, transform2DList[x].GameObjectPosition.y, 0.0f));
            spriteList[x].LastSpritePosition = spriteList[x].SpritePosition;
        }
        if (spriteList[x].LastSpriteRotation != spriteList[x].SpriteRotation) {
            spriteMatrix = glm::rotate(spriteMatrix, glm::radians(transform2DList[x].GameObjectRotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
            spriteMatrix = glm::rotate(spriteMatrix, glm::radians(transform2DList[x].GameObjectRotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
            spriteMatrix = glm::rotate(spriteMatrix, glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            spriteList[x].LastSpriteRotation = spriteList[x].SpriteRotation;
        }
        if (spriteList[x].LastSpriteScale != spriteList[x].SpriteScale) {
            spriteMatrix = glm::scale(spriteMatrix, glm::vec3(transform2DList[x].GameObjectScale.x, transform2DList[x].GameObjectScale.y, 1.0f));
            spriteList[x].LastSpriteScale = spriteList[x].SpriteScale;
        }

        spriteList[x].CurrentFrameTime += deltaTime;
        if (spriteList[x].CurrentFrameTime >= animationList[x].FrameHoldTime)
        {
            spriteList[x].CurrentFrame += 1;
            spriteList[x].CurrentFrameTime = 0.0f;
            if (spriteList[x].CurrentFrame >= spriteCount)
            {
                spriteList[x].CurrentFrame = 0;
            }
        }

        const ivec2& currentFrame = frameList[spriteList[x].CurrentFrame];
        spriteInstanceList[x].SpritePosition = transform2DList[x].GameObjectPosition;
        spriteInstanceList[x].SpriteSize = vramList[x].SpriteSize;
        spriteInstanceList[x].MaterialID = materialList[x].ShaderMaterialBufferIndex;
        spriteInstanceList[x].InstanceTransform = spriteMatrix;
        spriteInstanceList[x].FlipSprite = spriteList[x].FlipSprite;
        spriteInstanceList[x].UVOffset = glm::vec4(vramList[x].SpriteUVSize.x * currentFrame.x, vramList[x].SpriteUVSize.y * currentFrame.y, vramList[x].SpriteUVSize.x, vramList[x].SpriteUVSize.y);
    }
}

SpriteInstanceStruct Sprite_UpdateSprites(const Transform2DComponent& transform2D, const SpriteVram& vram, const Animation2D& animation, const Material& material, const ivec2& currentFrame, Sprite& sprite, size_t frameCount, float deltaTime)
{
    mat4 spriteMatrix = mat4(1.0f);
    if (sprite.LastSpritePosition != sprite.SpritePosition)
    {
        spriteMatrix = glm::translate(spriteMatrix, vec3(transform2D.GameObjectPosition.x, transform2D.GameObjectPosition.y, 0.0f));
        sprite.LastSpritePosition == sprite.SpritePosition;
    }
    if (sprite.LastSpriteRotation != sprite.SpriteRotation)
    {
        spriteMatrix = glm::rotate(spriteMatrix, glm::radians(transform2D.GameObjectRotation.x), vec3(1.0f, 0.0f, 0.0f));
        spriteMatrix = glm::rotate(spriteMatrix, glm::radians(transform2D.GameObjectRotation.y), vec3(0.0f, 1.0f, 0.0f));
        spriteMatrix = glm::rotate(spriteMatrix, glm::radians(0.0f), vec3(0.0f, 0.0f, 1.0f));
        sprite.LastSpriteRotation == sprite.SpriteRotation;
    }
    if (sprite.LastSpriteScale != sprite.SpriteScale)
    {
        spriteMatrix = glm::scale(spriteMatrix, vec3(transform2D.GameObjectScale.x, transform2D.GameObjectScale.y, 1.0f));
        sprite.LastSpriteScale == sprite.SpriteScale;
    }

    sprite.CurrentFrameTime += deltaTime;
    if (sprite.CurrentFrameTime >= animation.FrameHoldTime) {
        sprite.CurrentFrame += 1;
        sprite.CurrentFrameTime = 0.0f;
        if (sprite.CurrentFrame >= frameCount)
        {
            sprite.CurrentFrame = 0;
        }
    }

    SpriteInstanceStruct spriteInstance;
    spriteInstance.SpritePosition = transform2D.GameObjectPosition;
    spriteInstance.SpriteSize = vram.SpriteSize;
    spriteInstance.MaterialID = material.ShaderMaterialBufferIndex;
    spriteInstance.InstanceTransform = spriteMatrix;
    spriteInstance.FlipSprite = sprite.FlipSprite;
    spriteInstance.UVOffset = vec4(vram.SpriteUVSize.x * currentFrame.x, vram.SpriteUVSize.y * currentFrame.y, vram.SpriteUVSize.x, vram.SpriteUVSize.y);

    return spriteInstance;
}

void Sprite_SetSpriteAnimation(Sprite& sprite, Sprite::SpriteAnimationEnum spriteAnimation)
{
    if (sprite.CurrentAnimationID == spriteAnimation)
    {
        return;
    }

    sprite.CurrentAnimationID = spriteAnimation;
    sprite.CurrentFrame = 0;
    sprite.CurrentFrameTime = 0.0f;
}
