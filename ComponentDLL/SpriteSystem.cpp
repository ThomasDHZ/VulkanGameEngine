#include "pch.h"
#include "SpriteSystem.h"
#include "LevelSystem.h"

SpriteSystem& spriteSystem = SpriteSystem::Get();
uint32 SpriteSystem::SpriteIdd = 0;

uint32 SpriteSystem::GetNextSpriteIndex()
{
    return SpriteIdd++;
}

void SpriteSystem::AddSpriteBatchLayer()
{
    if (SpriteMeshId != UINT32_MAX)
    {
        Vector<Vertex2DLayout> spriteVertexList =
        {
            Vertex2DLayout(vec2(0.0f, 1.0f), vec2(0.0f, 0.0f)),
            Vertex2DLayout(vec2(1.0f, 1.0f), vec2(1.0f, 0.0f)),
            Vertex2DLayout(vec2(1.0f, 0.0f), vec2(1.0f, 1.0f)),
            Vertex2DLayout(vec2(0.0f, 0.0f), vec2(0.0f, 1.0f))
        };

        Vector<uint32> spriteIndexList =
        {
            0, 3, 1,
            1, 3, 2
        };

        VertexLayout vertexData =
        {
            .VertexType = VertexLayoutEnum::kVertexLayout_SpriteInstanceVertex,
            .VertexDataSize = sizeof(Vertex2DLayout) * spriteVertexList.size(),
            .VertexData = spriteVertexList.data(),
        };

        SpriteMeshId = meshSystem.CreateMesh("__SpriteMesh__", kMesh_SpriteMesh, vertexData, spriteIndexList);
    }

    SpriteLayerList.emplace_back(SpriteLayer
        {
            .InstanceCount = 0,
            .StartInstanceIndex = 0,
            .SpriteDrawLayer = UINT32_MAX,
        });
}

void SpriteSystem::CreateSprite(GameObject& gameObject, VkGuid& spriteVramId)
{
    Sprite sprite = levelSystem.EntityRegistry.emplace<Sprite>(gameObjectSystem.GameObjectComponentList[gameObject.GameObjectComponentIndex], Sprite
        {
            .SpriteId = static_cast<uint32>(GetNextSpriteIndex()),
            .GameObjectId = gameObject.GameObjectId,
            .SpriteInstanceId = memoryPoolSystem.AllocateObject(kSpriteInstanceBuffer),
            .CurrentAnimationId = 0,
            .CurrentFrame = 0,
            .SpriteLayer = FindSpriteVram(spriteVramId).SpriteLayer,
            .FlipSprite = ivec2(0),
            .SpriteVramId = spriteVramId,
            .CurrentFrameTime = 0.0f,
            .SpriteAlive = true,
            .IsSpriteTranformDirty = true,
            .IsSpriteAnimationDirty = true,
            .IsSpritePropertiesDirty = true,
        });
    SpriteInstance& spriteInstance = memoryPoolSystem.UpdateSpriteInstance(sprite.SpriteInstanceId);
    spriteInstance = SpriteInstance();

    AddSpriteBatchLayer();
    SpriteListDirty = true;
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
    const Texture& texture = textureSystem.FindTexture(material.AlbedoDataId);
    spriteSystem.SpriteAnimationMap[vramId] = vramSystem.LoadSpriteAnimations(spriteVramPath.c_str());
    spriteSystem.SpriteVramList.emplace_back(vramSystem.LoadSpriteVRAM(spriteVramPath.c_str(), material, texture));
    return vramId;
}

void SpriteSystem::Update(const float& deltaTime)
{
  // DestroyDeadSprites();
    SortSpriteLayers();
    auto view = levelSystem.EntityRegistry.view<GameObjectComponentLinker, Sprite, Transform2DComponent>();
    for (auto [entity, gameObjectId, sprite, transform] : view.each())
    {
        const auto& vram = FindSpriteVram(sprite.SpriteVramId);
        SpriteInstance& spriteInstance = memoryPoolSystem.UpdateSpriteInstance(sprite.SpriteInstanceId);
        if (sprite.IsSpriteTranformDirty)
        {
            GameObject& gameObject = gameObjectSystem.FindGameObject(sprite.GameObjectId);

            mat4 spriteMatrix = mat4(1.0f);
            spriteMatrix = glm::translate(spriteMatrix, vec3(transform.GameObjectPosition.x, transform.GameObjectPosition.y, 0.0f));
            spriteMatrix = glm::rotate(spriteMatrix, glm::radians(transform.GameObjectRotation.x), vec3(1.0f, 0.0f, 0.0f));
            spriteMatrix = glm::rotate(spriteMatrix, glm::radians(transform.GameObjectRotation.y), vec3(0.0f, 1.0f, 0.0f));
            spriteMatrix = glm::rotate(spriteMatrix, glm::radians(0.0f), vec3(0.0f, 0.0f, 1.0f));
            spriteMatrix = glm::scale(spriteMatrix, vec3(transform.GameObjectScale.x, transform.GameObjectScale.y, 1.0f));
            spriteInstance.SpritePosition = transform.GameObjectPosition;
            spriteInstance.InstanceTransform = spriteMatrix;
        }
        spriteInstance.SpriteSize = vram.SpriteSize;
        spriteInstance.FlipSprite = sprite.FlipSprite;
        //spriteInstance.Color = materialSystem.FindMaterialPoolIndex(vram.SpriteMaterialID);

        sprite.CurrentFrameTime += deltaTime;
        const auto& animation = FindSpriteAnimation(vram.VramSpriteID, sprite.CurrentAnimationId);
        const auto& currentFrame = animation.FrameList[sprite.CurrentFrame];
        if (sprite.CurrentFrameTime >= animation.FrameHoldTime)
        {
            sprite.CurrentFrame += 1;
            sprite.CurrentFrameTime = 0.0f;
            if (sprite.CurrentFrame >= animation.FrameList.size())
            {
                sprite.CurrentFrame = 0;
            }
            spriteInstance.UVOffset = vec4(vram.SpriteUVSize.x * currentFrame.x, vram.SpriteUVSize.y * currentFrame.y, vram.SpriteUVSize.x, vram.SpriteUVSize.y);
        }
    }
}

void SpriteSystem::SortSpriteLayers()
{
    struct SpriteSortStruct 
    {
        entt::entity entity;
        uint32       layer;
    };
    auto view = levelSystem.EntityRegistry.view<Sprite, Transform2DComponent>();

    Vector<SpriteSortStruct> entries;
    entries.reserve(view.size_hint());
    for (auto [entity, sprite, transform] : view.each())
    {
        if (sprite.SpriteAlive)
        {
            entries.push_back({ entity, sprite.SpriteLayer });
        }
    }
    std::stable_sort(entries.begin(), entries.end(), [](const SpriteSortStruct& a, const SpriteSortStruct& b)
        {
            return a.layer < b.layer;
        });

    for (auto& layer : SpriteLayerList)
    {
        layer.StartInstanceIndex = 0;
        layer.InstanceCount = 0;
    }

    uint32 currentInstanceIndex = 0;
    for (const auto& entry : entries)
    {
        bool spriteLayerExists = false;
        auto [sprite, transform] = levelSystem.EntityRegistry.get<Sprite, Transform2DComponent>(entry.entity);
        for (auto& layer : SpriteLayerList)
        {
            if (layer.SpriteDrawLayer == sprite.SpriteLayer)
            {
                if (layer.InstanceCount == 0) layer.StartInstanceIndex = currentInstanceIndex;
                layer.InstanceCount++;
                spriteLayerExists = true;
                break;
            }
        }
        if (!spriteLayerExists)
        {
            SpriteLayer newLayer;
            newLayer.SpriteDrawLayer = sprite.SpriteLayer;
            newLayer.StartInstanceIndex = currentInstanceIndex;
            newLayer.InstanceCount = 1;
            SpriteLayerList.push_back(newLayer);
        }
        currentInstanceIndex++;
    }
}

Animation2D& SpriteSystem::FindSpriteAnimation(const VramSpriteGuid& vramId, const AnimationListId& animationId)
{
    return spriteSystem.SpriteAnimationMap.at(vramId)[animationId];
}

SpriteVram& SpriteSystem::FindSpriteVram(VramSpriteGuid vramSpriteId)
{
    return *std::find_if(spriteSystem.SpriteVramList.begin(), spriteSystem.SpriteVramList.end(), [vramSpriteId](const SpriteVram& sprite) { return sprite.VramSpriteID == vramSpriteId; });
}

void SpriteSystem::Destroy(Sprite& sprite)
{

}

void SpriteSystem::SetSpriteAnimation(Sprite* sprite, uint spriteAnimationEnum)
{
    if (sprite->CurrentAnimationId == spriteAnimationEnum)
    {
        return;
    }

    sprite->CurrentAnimationId = spriteAnimationEnum;
    sprite->CurrentFrame = 0;
    sprite->CurrentFrameTime = 0.0f;
}