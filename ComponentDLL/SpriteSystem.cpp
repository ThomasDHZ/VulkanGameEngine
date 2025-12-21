#include "pch.h"
#include "SpriteSystem.h"
#include "Vertex.h"

SpriteSystem& spriteSystem = SpriteSystem::Get();

uint32 SpriteSystem::GetNextSpriteIndex()
{
    if (!spriteSystem.FreeSpriteIndicesList.empty())
    {
        uint index = spriteSystem.FreeSpriteIndicesList.back();
        spriteSystem.FreeSpriteIndicesList.pop_back();
        return index;
    }
    return spriteSystem.SpriteList.size();
}

void SpriteSystem::AddSpriteBatchLayer(RenderPassGuid& renderPassId, uint32 spriteDrawLayer)
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
        .SpriteLayerMeshId = meshSystem.CreateMesh(MeshTypeEnum::Mesh_SpriteMesh, SpriteVertexList, SpriteIndexList)
    };

    Vector<SpriteInstance> spriteInstanceList = spriteSystem.FindSpriteInstancesByLayer(spriteLayer);
    spriteLayer.SpriteLayerBufferId = bufferSystem.VMACreateVulkanBuffer<SpriteInstance>(spriteInstanceList, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, false);
    spriteSystem.SpriteLayerList[spriteDrawLayer] = spriteLayer;
}

VramSpriteGuid SpriteSystem::LoadSpriteVRAM(const String& spriteVramPath)
{
    nlohmann::json json = fileSystem.LoadJsonFile(spriteVramPath.c_str());
    VramSpriteGuid vramId = VramSpriteGuid(json["VramSpriteId"].get<String>().c_str());
    if (vramSystem.SpriteVramExists(vramId))
    {
        return vramId;
    }

    VramSpriteGuid materialId = VramSpriteGuid(json["MaterialId"].get<String>().c_str());
    const Material& material = materialSystem.FindMaterial(materialId);
    const Texture& texture = textureSystem.FindTexture(material.AlbedoMapId);
    spriteSystem.SpriteAnimationMap[vramId] = vramSystem.LoadSpriteAnimations(spriteVramPath.c_str());
    spriteSystem.SpriteVramList.emplace_back(vramSystem.LoadSpriteVRAM(spriteVramPath.c_str(), material, texture));
    return vramId;
}

void SpriteSystem::UpdateSprites(const float& deltaTime)
{
    for (auto& sprite : spriteSystem.SpriteList)
    {
        const auto& transform2D = gameObjectSystem.FindTransform2DComponent(sprite.GameObjectId);
        const auto& vram = FindSpriteVram(sprite.SpriteVramId);
        const auto& animation = FindSpriteAnimation(vram.VramSpriteID, sprite.CurrentAnimationID);
        const auto& material = materialSystem.FindMaterial(vram.SpriteMaterialID);
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
            if (sprite.CurrentFrame >= animation.FrameList.size())
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

void SpriteSystem::UpdateSpriteBatchLayers(const float& deltaTime)
{
    for (auto& spriteLayer : SpriteLayerList)
    {
        Vector<SpriteInstance> spriteInstanceList = FindSpriteInstancesByLayer(spriteLayer.second);
        bufferSystem.VMAUpdateDynamicBuffer(spriteLayer.second.SpriteLayerBufferId, spriteInstanceList.data(), sizeof(SpriteInstance) * spriteInstanceList.size());
    }
}

void SpriteSystem::UpdateBatchSprites(SpriteInstance* spriteInstanceList, Sprite* spriteList, const Transform2DComponent* transform2DList, const SpriteVram* vramList, const Animation2D* animationList, const Material* materialList, size_t spriteCount, float deltaTime)
{
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

        const ivec2& currentFrame = animationList->FrameList[spriteList[x].CurrentFrame];
        spriteInstanceList[x].SpritePosition = transform2DList[x].GameObjectPosition;
        spriteInstanceList[x].SpriteSize = vramList[x].SpriteSize;
        spriteInstanceList[x].MaterialID = materialList[x].ShaderMaterialBufferIndex;
        spriteInstanceList[x].InstanceTransform = spriteMatrix;
        spriteInstanceList[x].FlipSprite = spriteList[x].FlipSprite;
        spriteInstanceList[x].UVOffset = glm::vec4(vramList[x].SpriteUVSize.x * currentFrame.x, vramList[x].SpriteUVSize.y * currentFrame.y, vramList[x].SpriteUVSize.x, vramList[x].SpriteUVSize.y);
    }
}

Vector<SpriteInstance> SpriteSystem::FindSpriteInstancesByLayer(const SpriteLayer& spriteLayer)
{
    Vector<SpriteInstance> spriteInstanceList;
    Vector<Sprite> spriteList = FindSpritesByLayer(spriteLayer);
    for (auto& sprite : spriteList)
    {
        spriteInstanceList.emplace_back(spriteSystem.SpriteInstanceList[sprite.SpriteInstance]);
    }

    return spriteInstanceList;
}

SpriteVram& SpriteSystem::FindSpriteVram(VramSpriteGuid vramSpriteId)
{
    return *std::find_if(spriteSystem.SpriteVramList.begin(), spriteSystem.SpriteVramList.end(), [vramSpriteId](const SpriteVram& sprite) { return sprite.VramSpriteID == vramSpriteId; });
}

Animation2D& SpriteSystem::FindSpriteAnimation(const VramSpriteGuid& vramId, const AnimationListId& animationId)
{
    return spriteSystem.SpriteAnimationMap.at(vramId)[animationId];
}

void SpriteSystem::Destroy()
{

}

const bool SpriteSystem::SpriteLayerExists(const uint32 spriteDrawLayer)
{
    return spriteSystem.SpriteLayerList.contains(spriteDrawLayer);
}

void SpriteSystem::AddSprite(GameObject& gameObject, VkGuid& spriteVramId)
{
    Sprite sprite;
    sprite.SpriteID = GetNextSpriteIndex();
    sprite.GameObjectId = gameObject.GameObjectId;
    sprite.SpriteVramId = spriteVramId;
    sprite.SpriteLayer = FindSpriteVram(spriteVramId).SpriteLayer;
    sprite.SpriteInstance = spriteSystem.SpriteInstanceList.size();
    spriteSystem.SpriteList.emplace_back(sprite);
    spriteSystem.SpriteInstanceList.emplace_back(SpriteInstance());
    if (!SpriteLayerExists(sprite.SpriteLayer))
    {
        VkGuid guid = VkGuid("de78235b-3c69-4298-99da-acd8c6622ece");
        AddSpriteBatchLayer(guid, sprite.SpriteLayer);
    }
}

void SpriteSystem::Update(float deltaTime)
{
    VkCommandBuffer commandBuffer = renderSystem.BeginSingleUseCommand(vulkanSystem.CommandPool);
    UpdateSprites(deltaTime);
    UpdateSpriteBatchLayers(deltaTime);
    renderSystem.EndSingleUseCommand(commandBuffer);
}

void SpriteSystem::SetSpriteAnimation(Sprite* sprite, uint spriteAnimationEnum)
{
    if (sprite->CurrentAnimationID == spriteAnimationEnum)
    {
        return;
    }

    sprite->CurrentAnimationID = spriteAnimationEnum;
    sprite->CurrentFrame = 0;
    sprite->CurrentFrameTime = 0.0f;
}

Sprite SpriteSystem::FindSprite(uint gameObjectId)
{
    auto it = std::find_if(spriteSystem.SpriteList.begin(), spriteSystem.SpriteList.end(), [gameObjectId](const Sprite& sprite) { return sprite.GameObjectId == gameObjectId; });
    if (it != spriteSystem.SpriteList.end()) 
    {
        return *it; 
    }
    else 
    {
        return Sprite{};
    }
}

Vector<Sprite> SpriteSystem::FindSpritesByLayer(const SpriteLayer& spriteLayer)
{
    Vector<Sprite> matches;
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
