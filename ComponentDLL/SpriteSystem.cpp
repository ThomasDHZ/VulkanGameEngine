#include "pch.h"
#include "SpriteSystem.h"
#include "FileSystem.h"
#include "VulkanBuffer.h"
#include "BufferSystem.h"
#include "TextureSystem.h"
#include "MaterialSystem.h"
#include <MeshSystem.h>
#include "GameObject.h"
#include "Vertex.h"

SpriteSystem spriteSystem = SpriteSystem();

uint32 GetNextSpriteIndex()
{
    if (!spriteSystem.FreeSpriteIndicesList.empty())
    {
        uint index = spriteSystem.FreeSpriteIndicesList.back();
        spriteSystem.FreeSpriteIndicesList.pop_back();
        return index;
    }
    return spriteSystem.SpriteList.size();
}

void Sprite_UpdateBatchSprites(SpriteInstance* spriteInstanceList, Sprite* spriteList, const Transform2DComponent* transform2DList, const SpriteVram* vramList, const Animation2D* animationList, const Material* materialList, size_t spriteCount, float deltaTime)
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

void Sprite_SetSpriteAnimation(Sprite& sprite, uint spriteAnimationEnum)
{
    if (sprite.CurrentAnimationID == spriteAnimationEnum)
    {
        return;
    }

    sprite.CurrentAnimationID = spriteAnimationEnum;
    sprite.CurrentFrame = 0;
    sprite.CurrentFrameTime = 0.0f;
}

void Sprite_UpdateSprites(const float& deltaTime)
{
    for (auto& sprite : spriteSystem.SpriteList)
    {
        const auto& transform2D = GameObject_FindTransform2DComponent(sprite.GameObjectId);
        const auto& vram = Sprite_FindSpriteVram(sprite.SpriteVramId);
        const auto& animation = Sprite_FindSpriteAnimation(vram.VramSpriteID, sprite.CurrentAnimationID);
        const auto& material = Material_FindMaterial(vram.SpriteMaterialID);
        const auto& currentFrame = animation.FrameList[sprite.CurrentFrame];
        
        mat4 spriteMatrix = mat4(1.0f);
        if (sprite.LastSpritePosition != sprite.SpritePosition)
        {
            spriteMatrix = glm::translate(spriteMatrix, vec3(transform2D.GameObjectPosition.x, transform2D.GameObjectPosition.y, 0.0f));
            sprite.LastSpritePosition = sprite.SpritePosition;
        }
        if (sprite.LastSpriteRotation != sprite.SpriteRotation)
        {
            spriteMatrix = glm::rotate(spriteMatrix, glm::radians(transform2D.GameObjectRotation.x), vec3(1.0f, 0.0f, 0.0f));
            spriteMatrix = glm::rotate(spriteMatrix, glm::radians(transform2D.GameObjectRotation.y), vec3(0.0f, 1.0f, 0.0f));
            spriteMatrix = glm::rotate(spriteMatrix, glm::radians(0.0f), vec3(0.0f, 0.0f, 1.0f));
            sprite.LastSpriteRotation = sprite.SpriteRotation;
        }
        if (sprite.LastSpriteScale != sprite.SpriteScale)
        {
            spriteMatrix = glm::scale(spriteMatrix, vec3(transform2D.GameObjectScale.x, transform2D.GameObjectScale.y, 1.0f));
            sprite.LastSpriteScale = sprite.SpriteScale;
        }

        sprite.CurrentFrameTime += deltaTime;
        if (sprite.CurrentFrameTime >= animation.FrameHoldTime) {
            sprite.CurrentFrame += 1;
            sprite.CurrentFrameTime = 0.0f;
            if (sprite.CurrentFrame >= animation.FrameCount)
            {
                sprite.CurrentFrame = 0;
            }
        }

        SpriteInstance spriteInstance;
        spriteInstance.SpritePosition = transform2D.GameObjectPosition;
        spriteInstance.SpriteSize = vram.SpriteSize;
        spriteInstance.MaterialID = material.ShaderMaterialBufferIndex;
        spriteInstance.InstanceTransform = spriteMatrix;
        spriteInstance.FlipSprite = sprite.FlipSprite;
        spriteInstance.UVOffset = vec4(vram.SpriteUVSize.x * currentFrame.x, vram.SpriteUVSize.y * currentFrame.y, vram.SpriteUVSize.x, vram.SpriteUVSize.y);

        spriteSystem.SpriteInstanceList[sprite.SpriteInstance] = spriteInstance;
    }
}

void Sprite_UpdateSpriteBatchLayers(const GraphicsRenderer& renderer, const float& deltaTime)
{
    for (auto& spriteLayer : spriteSystem.SpriteLayerList)
    {
        Vector<SpriteInstance> spriteInstanceList = Sprite_FindSpriteInstancesByLayer(spriteLayer.second);
        bufferSystem.UpdateBufferMemory(renderer, spriteLayer.second.SpriteLayerBufferId, spriteInstanceList);
    }
}

void Sprite_AddSprite(GameObject& gameObject, VkGuid& spriteVramId)
{
    Sprite sprite;
    sprite.SpriteID = GetNextSpriteIndex();
    sprite.GameObjectId = gameObject.GameObjectId;
    sprite.SpriteVramId = spriteVramId;
    sprite.SpriteLayer = Sprite_FindSpriteVram(spriteVramId).SpriteLayer;
    sprite.SpriteInstance = spriteSystem.SpriteInstanceList.size();
    spriteSystem.SpriteList.emplace_back(sprite);
    spriteSystem.SpriteInstanceList.emplace_back(SpriteInstance());
    if (!Sprite_SpriteLayerExists(sprite.SpriteLayer))
    {
        VkGuid guid = VkGuid("de78235b-3c69-4298-99da-acd8c6622ece");
        Sprite_AddSpriteBatchLayer(renderer, guid, sprite.SpriteLayer);
    }
}

void Sprite_AddSpriteBatchLayer(const GraphicsRenderer& renderer, RenderPassGuid& renderPassId, uint32 spriteDrawLayer)
{
    const VkBufferUsageFlags MeshBufferUsageSettings = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT |
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
        VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    const VkMemoryPropertyFlags MeshBufferPropertySettings = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    Vector<Vertex2D> SpriteVertexList =
    {
        Vertex2D(vec2(0.0f, 1.0f), vec2(0.0f, 0.0f)),
        Vertex2D(vec2(1.0f, 1.0f), vec2(1.0f, 0.0f)),
        Vertex2D(vec2(1.0f, 0.0f), vec2(1.0f, 1.0f)),
        Vertex2D(vec2(0.0f, 0.0f), vec2(0.0f, 1.0f))
    };

    Vector<uint32> SpriteIndexList =
    {
        0, 3, 1,
        1, 3, 2
    };

    SpriteLayer spriteLayer = SpriteLayer
    {
        .RenderPassId = renderPassId,
        .SpriteDrawLayer = spriteDrawLayer,
        .SpriteLayerMeshId = Mesh_CreateMesh(renderer, MeshTypeEnum::Mesh_SpriteMesh, SpriteVertexList, SpriteIndexList)
    };

    Vector<SpriteInstance> spriteInstanceList = Sprite_FindSpriteInstancesByLayer(spriteLayer);
    spriteLayer.SpriteLayerBufferId = bufferSystem.CreateVulkanBuffer<SpriteInstance>(renderer, spriteInstanceList, MeshBufferUsageSettings, MeshBufferPropertySettings, false);
    spriteSystem.SpriteLayerList[spriteDrawLayer] = spriteLayer;
}

void Sprite_Update(const GraphicsRenderer& renderer, const float& deltaTime)
{
    VkCommandBuffer commandBuffer = Renderer_BeginSingleTimeCommands(renderer.Device, renderer.CommandPool);
    Sprite_UpdateSprites(deltaTime);
    Sprite_UpdateSpriteBatchLayers(renderer, deltaTime);
    Renderer_EndSingleTimeCommands(renderer.Device, renderer.CommandPool, renderer.GraphicsQueue, commandBuffer);
}

void Sprite_SetSpriteAnimation(Sprite* sprite, uint spriteAnimationEnum)
{
    Sprite_SetSpriteAnimation(*sprite, spriteAnimationEnum);
}

Sprite* Sprite_FindSprite(uint gameObjectId)
{
    auto it = std::find_if(spriteSystem.SpriteList.begin(), spriteSystem.SpriteList.end(), [gameObjectId](const Sprite& sprite) { return sprite.GameObjectId == gameObjectId; });
    return it != spriteSystem.SpriteList.end() ? &(*it) : nullptr;
}

Vector<std::reference_wrapper<Sprite>> Sprite_FindSpritesByLayer(const SpriteLayer& spriteLayer)
{
    Vector<std::reference_wrapper<Sprite>> matches;
    auto it = spriteSystem.SpriteList.begin();
    while ((it = std::find_if(it, spriteSystem.SpriteList.end(), [spriteLayer](const Sprite& sprite)
        {
            return sprite.SpriteLayer == spriteLayer.SpriteDrawLayer;
        })) != spriteSystem.SpriteList.end()) {
        matches.emplace_back(std::ref(*it));
        ++it;
    }
    return matches;
}

const Vector<Mesh>& Sprite_FindSpriteLayerMeshList()
{
    return Mesh_FindMeshByMeshType(MeshTypeEnum::Mesh_SpriteMesh);
}

Vector<SpriteInstance> Sprite_FindSpriteInstancesByLayer(const SpriteLayer& spriteLayer)
{
    Vector<SpriteInstance> spriteInstanceList;
    Vector<std::reference_wrapper<Sprite>> spriteList = Sprite_FindSpritesByLayer(spriteLayer);
    for (auto& sprite : spriteList)
    {
        spriteInstanceList.emplace_back(spriteSystem.SpriteInstanceList[sprite.get().SpriteInstance]);
    }
    return spriteInstanceList;
}

SpriteVram& Sprite_FindSpriteVram(VkGuid vramSpriteId)
{
    return *std::find_if(spriteSystem.SpriteVramList.begin(), spriteSystem.SpriteVramList.end(), [vramSpriteId](const SpriteVram& sprite) { return sprite.VramSpriteID == vramSpriteId; });
}

Animation2D& Sprite_FindSpriteAnimation(const VramSpriteGuid& vramId, const UM_AnimationListID& animationId)
{
    return spriteSystem.SpriteAnimationMap.at(vramId)[animationId];
}

VkGuid Sprite_LoadSpriteVRAM(const String& spriteVramPath)
{
    nlohmann::json json = File_LoadJsonFile(spriteVramPath.c_str());
    VkGuid vramId = VkGuid(json["VramSpriteId"].get<String>().c_str());
    if (VRAM_SpriteVramExists(vramId))
    {
        return vramId;
    }

    size_t animationListCount = 0;
    VkGuid materialId = VkGuid(json["MaterialId"].get<String>().c_str());
    const Material& material = Material_FindMaterial(materialId);
    const Texture& texture = Texture_FindTexture(material.AlbedoMapId);
    Animation2D* animationListPtr = VRAM_LoadSpriteAnimations(spriteVramPath.c_str(), animationListCount);
    spriteSystem.SpriteAnimationMap[vramId] = Vector<Animation2D>(animationListPtr, animationListPtr + animationListCount);
    spriteSystem.SpriteVramList.emplace_back(VRAM_LoadSpriteVRAM(spriteVramPath.c_str(), material, texture));
    memorySystem.RemovePtrBuffer(animationListPtr);
    return vramId;
}

void Sprite_Destroy()
{

}

const bool Sprite_SpriteLayerExists(const uint32 spriteDrawLayer)
{
    return spriteSystem.SpriteLayerList.contains(spriteDrawLayer);
}