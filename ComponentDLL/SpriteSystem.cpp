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

//entt::entity SpriteSystem::CreateSprite(entt::registry& registry, VkGuid spriteVramId, vec2 pos, int layer, vec4 tint)
//{
//    entt::entity entity = registry.create();
//    registry.emplace<SpriteComponent>(entity, spriteVramId, 0, 0, 0.0f, false, false, layer, tint);
//    registry.emplace<Transform2DComponent>(entity, pos, 0.0f, glm::vec2(1.0f));
//    registry.emplace<RenderableTag>(entity);
//    return entity;
//}
//
//void UpdateSprites(entt::registry& registry, float deltaTime)
//{
//    registry.view<SpriteComponent>().each([&](SpriteComponent& sprite)
//        {
//            sprite.frameTimeAccumulator += deltaTime;
//            const SpriteVram& vram = spriteSystem.FindSpriteVram(sprite.spriteVramId);
//            const Animation2D& anim = spriteSystem.FindSpriteAnimation(vram.VramSpriteID, sprite.currentAnimationId);
//            const AnimationFrames& frame = anim.FrameList[sprite.currentFrame];
//            if (s.frameTimeAccumulator >= frame.duration) 
//            {
//                s.currentFrame = (s.currentFrame + 1) % anim.FrameList.size();
//                s.frameTimeAccumulator = 0.0f;
//            }
//        });
//}
void SpriteSystem::AddSprite(GameObject& gameObject, VkGuid& spriteVramId)
{
    Sprite sprite = levelSystem.EntityRegistry.emplace<Sprite>(gameObjectSystem.GameObjectComponentList[gameObject.GameObjectComponentIndex], Sprite
        {
            .SpriteId = static_cast<uint32>(GetNextSpriteIndex()),
            .GameObjectId = gameObject.GameObjectId,
            .SpriteInstanceId = memoryPoolSystem.AllocateObject(kSpriteInstanceBuffer),
            .CurrentAnimationId = 0,
            .CurrentFrame = 0,
            .SpriteLayer = FindSpriteVram(spriteVramId).SpriteLayer,
            .SpritePosition = vec2(0.0f),
            .SpriteRotation = vec2(0.0f),
            .SpriteScale = vec2(1.0f),
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
    DestroyDeadSprites();
    SyncSpritesWithSpriteInstances();
    auto view = levelSystem.EntityRegistry.view<GameObjectComponentLinker, Sprite, Transform2DComponent>();
    for (auto [entity, gameObjectId, sprite, transform] : view.each())
    {
        if (!sprite.IsSpriteTranformDirty &&
            !sprite.IsSpriteAnimationDirty &&
            !sprite.IsSpritePropertiesDirty)
        {
            continue;
        }

        SpriteInstance& spriteInstance = memoryPoolSystem.UpdateSpriteInstance(sprite.SpriteInstanceId);
        if (sprite.IsSpriteTranformDirty)
        {
            GameObject& gameObject = gameObjectSystem.FindGameObject(sprite.GameObjectId);
            entt::entity entity = gameObjectSystem.GameObjectComponentList[gameObject.GameObjectComponentIndex];
            if (transform.GameObjectId != sprite.GameObjectId) continue;

            mat4 spriteMatrix = mat4(1.0f);
            spriteMatrix = glm::translate(spriteMatrix, vec3(transform.GameObjectPosition.x, transform.GameObjectPosition.y, 0.0f));
            spriteMatrix = glm::rotate(spriteMatrix, glm::radians(transform.GameObjectRotation.x), vec3(1.0f, 0.0f, 0.0f));
            spriteMatrix = glm::rotate(spriteMatrix, glm::radians(transform.GameObjectRotation.y), vec3(0.0f, 1.0f, 0.0f));
            spriteMatrix = glm::rotate(spriteMatrix, glm::radians(0.0f), vec3(0.0f, 0.0f, 1.0f));
            spriteMatrix = glm::scale(spriteMatrix, vec3(transform.GameObjectScale.x, transform.GameObjectScale.y, 1.0f));
            spriteInstance.SpritePosition = transform.GameObjectPosition;
            spriteInstance.InstanceTransform = spriteMatrix;

        }

        const auto& vram = FindSpriteVram(sprite.SpriteVramId);
        if (sprite.IsSpritePropertiesDirty)
        {
            spriteInstance.SpriteSize = vram.SpriteSize;
            spriteInstance.FlipSprite = sprite.FlipSprite;
            //spriteInstance.Color = materialSystem.FindMaterialPoolIndex(vram.SpriteMaterialID);
        }

        if (sprite.IsSpriteAnimationDirty)
        {
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
}

void SpriteSystem::SyncSpritesWithSpriteInstances()
{
    // 1. Collect living sprites from EnTT
    struct SortEntry {
        entt::entity entity;
        uint32_t     layer;
    };

    auto view = levelSystem.EntityRegistry.view<Sprite, Transform2DComponent>();

    std::vector<SortEntry> entries;
    entries.reserve(view.size_hint());

    for (auto [entity, sprite, transform] : view.each())
    {
        if (sprite.SpriteAlive)
        {
            entries.push_back({ entity, sprite.SpriteLayer });
        }
    }

    printf("Collected %zu living sprites for sorting\n", entries.size());

    // 2. Stable sort by layer
    std::stable_sort(entries.begin(), entries.end(), [](const SortEntry& a, const SortEntry& b) 
        {
            return a.layer < b.layer;
        });

    // 3. Reset layer buckets (keep this if render still uses SpriteLayerList)
    for (auto& layer : SpriteLayerList)
    {
        layer.StartInstanceIndex = 0;
        layer.InstanceCount = 0;
    }

    // 4. Re-fill layer buckets + write instance data sequentially
    uint32_t currentInstanceIndex = 0;
    for (const auto& entry : entries)
    {
        auto [sprite, transform] = levelSystem.EntityRegistry.get<Sprite, Transform2DComponent>(entry.entity);
        SpriteInstance& instanceBuffer = memoryPoolSystem.UpdateSpriteInstance(currentInstanceIndex); // assume this maps the buffer

        // Fill layer bucket
        bool found = false;
        for (auto& layer : SpriteLayerList)
        {
            if (layer.SpriteDrawLayer == sprite.SpriteLayer)
            {
                if (layer.InstanceCount == 0)
                    layer.StartInstanceIndex = currentInstanceIndex;
                layer.InstanceCount++;
                found = true;
                break;
            }
        }
        if (!found)
        {
            // Add missing layer bucket
            SpriteLayer newLayer;
            newLayer.SpriteDrawLayer = sprite.SpriteLayer;
            newLayer.StartInstanceIndex = currentInstanceIndex;
            newLayer.InstanceCount = 1;
            SpriteLayerList.push_back(newLayer);
        }

        // Transform
        mat4 spriteMatrix = glm::translate(mat4(1.0f), vec3(transform.GameObjectPosition, 0.0f));
        spriteMatrix = glm::rotate(spriteMatrix, glm::radians(transform.GameObjectRotation.y), vec3(0, 1, 0));
        spriteMatrix = glm::rotate(spriteMatrix, glm::radians(transform.GameObjectRotation.x), vec3(1, 0, 0));
        spriteMatrix = glm::scale(spriteMatrix, vec3(transform.GameObjectScale, 1.0f));

        instanceBuffer.SpritePosition = transform.GameObjectPosition;
        instanceBuffer.InstanceTransform = spriteMatrix;

        // VRAM data
        const auto& vram = FindSpriteVram(sprite.SpriteVramId);
        instanceBuffer.SpriteSize = vram.SpriteSize;
        instanceBuffer.FlipSprite = sprite.FlipSprite;
        instanceBuffer.Color = vram.SpriteColor;
        instanceBuffer.MaterialId = materialSystem.FindMaterialPoolIndex(vram.SpriteMaterialID);

        // Animation UV (always set — safe and cheap)
        const auto& anim = FindSpriteAnimation(vram.VramSpriteID, sprite.CurrentAnimationId);
        const auto& frame = anim.FrameList[sprite.CurrentFrame];
        instanceBuffer.UVOffset = vec4(
            vram.SpriteUVSize.x * frame.x,
            vram.SpriteUVSize.y * frame.y,
            vram.SpriteUVSize.x,
            vram.SpriteUVSize.y
        );

        currentInstanceIndex++;
    }

    uint32 instanceStartIndex = 0;
    for (size_t x = 0; x < SpriteLayerList.size(); ++x)
    {
        SpriteLayerList[x].InstanceCount = SpriteLayerList[x].InstanceCount; // already set
        SpriteLayerList[x].StartInstanceIndex = instanceStartIndex;
        SpriteLayerList[x].SpriteDrawLayer = static_cast<uint32>(x);
        instanceStartIndex += SpriteLayerList[x].InstanceCount;
    }

    printf("Sync complete: %zu sorted sprites, %zu layer buckets\n",
        entries.size(), SpriteLayerList.size());
}

Animation2D& SpriteSystem::FindSpriteAnimation(const VramSpriteGuid& vramId, const AnimationListId& animationId)
{
    return spriteSystem.SpriteAnimationMap.at(vramId)[animationId];
}

SpriteVram& SpriteSystem::FindSpriteVram(VramSpriteGuid vramSpriteId)
{
    return *std::find_if(spriteSystem.SpriteVramList.begin(), spriteSystem.SpriteVramList.end(), [vramSpriteId](const SpriteVram& sprite) { return sprite.VramSpriteID == vramSpriteId; });
}

void SpriteSystem::DestroySprite(uint32 spriteId)
{
    //auto it = std::find_if(SpriteList.begin(), SpriteList.end(), [spriteId](const Sprite& sprite) 
    //    { 
    //        return sprite.SpriteId == spriteId; 
    //    });

    //if (it != SpriteList.end())
    //{
    //    Sprite& sprite = *it;
    //    if (sprite.SpriteInstanceId != UINT32_MAX)
    //    {
    //        memoryPoolSystem.FreeObject(kSpriteInstanceBuffer, sprite.SpriteInstanceId);
    //        sprite.SpriteInstanceId = UINT32_MAX;
    //    }
    //    sprite.SpriteAlive = false;
    //    SpriteListDirty = true;
    //}
}

void SpriteSystem::DestroyDeadSprites()
{
    Vector<Sprite> deadSpritesList;
    std::copy_if(deadSpritesList.begin(), deadSpritesList.end(), std::back_inserter(deadSpritesList), [](const Sprite& sprite)
        {
            return sprite.SpriteAlive == false;
        });

    for (auto& deadSprite : deadSpritesList)
    {
        DestroySprite(deadSprite.SpriteId);
    };
}

void SpriteSystem::Destroy()
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

void SpriteSystem::SortSpritesbyLayer()
{
 /*   if (SpriteListDirty)
    {
        auto pointers = memoryPoolSystem.GetActiveSpriteInstancePointers();
        std::stable_sort(pointers.begin(), pointers.end(), [](SpriteInstance* a, SpriteInstance* b)
            {
                return a->LayerId < b->LayerId;
            });

        uint32 newIndex = 0;
        for (SpriteInstance* ptr : pointers)
        {
            SpriteInstance& target = memoryPoolSystem.UpdateSpriteInstance(newIndex);
            target = *ptr;
            newIndex++;
        }
    }*/
}