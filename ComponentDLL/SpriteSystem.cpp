#include "pch.h"
#include "SpriteSystem.h"

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

void SpriteSystem::AddSprite(GameObject& gameObject, VkGuid& spriteVramId)
{

    spriteSystem.SpriteList.emplace_back(Sprite
        {
            .SpriteId = static_cast<uint32>(spriteSystem.SpriteList.size()),
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
    SpriteInstance& spriteInstance = memoryPoolSystem.UpdateSpriteInstance(spriteSystem.SpriteList.back().SpriteInstanceId);
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
    for (auto& sprite : spriteSystem.SpriteList)
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
            mat4 spriteMatrix = mat4(1.0f);
            const auto& transform2D = gameObjectSystem.FindTransform2DComponent(sprite.GameObjectId);   
            spriteMatrix = glm::translate(spriteMatrix, vec3(transform2D.GameObjectPosition.x, transform2D.GameObjectPosition.y, 0.0f));
            spriteMatrix = glm::rotate(spriteMatrix, glm::radians(transform2D.GameObjectRotation.x), vec3(1.0f, 0.0f, 0.0f));
            spriteMatrix = glm::rotate(spriteMatrix, glm::radians(transform2D.GameObjectRotation.y), vec3(0.0f, 1.0f, 0.0f));
            spriteMatrix = glm::rotate(spriteMatrix, glm::radians(0.0f), vec3(0.0f, 0.0f, 1.0f));
            spriteMatrix = glm::scale(spriteMatrix, vec3(transform2D.GameObjectScale.x, transform2D.GameObjectScale.y, 1.0f));
            spriteInstance.SpritePosition = transform2D.GameObjectPosition;
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
    if (!SpriteListDirty)
    {
        return;
    }

    SpriteListDirty = false;
    std::stable_sort(SpriteList.begin(), SpriteList.end(),
        [](const Sprite& a, const Sprite& b)
        {
            return a.SpriteLayer < b.SpriteLayer;
        });

    uint32 currentInstanceIndex = 0;
    for (auto& layer : SpriteLayerList)
    {
        layer.StartInstanceIndex = 0;
        layer.InstanceCount = 0;
    }

    for (auto& sprite : SpriteList)
    {
        if (!sprite.SpriteAlive)
        {
            continue;
        }

        if (sprite.SpriteInstanceId != currentInstanceIndex)
        {
            if (sprite.SpriteInstanceId != UINT32_MAX)
            {
                memoryPoolSystem.FreeObject(kSpriteInstanceBuffer, sprite.SpriteInstanceId);
            }
            sprite.SpriteInstanceId = currentInstanceIndex;
        }

        for (auto& layer : SpriteLayerList)
        {
            if (layer.SpriteDrawLayer == sprite.SpriteLayer)
            {
                if (layer.InstanceCount == 0)
                    layer.StartInstanceIndex = currentInstanceIndex;

                layer.InstanceCount++;
                break;
            }
        }
        currentInstanceIndex++;
    }

    auto oldSpriteListPtr = memoryPoolSystem.GetActiveSpriteInstancePointers();
    auto a = oldSpriteListPtr[0];
    auto oldSpriteList = SpriteList;
    Vector<SpriteInstance*> spriteInstancePtrList = memoryPoolSystem.GetActiveSpriteInstancePointers();
    for (int x = 0; x < SpriteList.size(); x++)
    {
        if (oldSpriteList[x].SpriteInstanceId == SpriteList[x].SpriteInstanceId)
        {
            continue;
        }
        else
        {
            SpriteInstance spriteInstance = *spriteInstancePtrList[x];
            *spriteInstancePtrList[x] = *spriteInstancePtrList[oldSpriteList[x].SpriteInstanceId];
            *spriteInstancePtrList[oldSpriteList[x].SpriteInstanceId] = *spriteInstancePtrList[x];
        }
    }

    uint32 instanceStartIndex = 0;
    for (int x = 0; x < SpriteLayerList.size(); x++)
    {
        uint32 layerCount = GetSpriteCountofLayer(x);
        SpriteLayerList[x] = SpriteLayer
        {
            .InstanceCount = layerCount,
            .StartInstanceIndex = instanceStartIndex,
            .SpriteDrawLayer = static_cast<uint32>(x),
        };
        instanceStartIndex += layerCount;
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

uint32 SpriteSystem::GetSpriteCountofLayer(uint32 layerId)
{
    return std::count_if(SpriteList.begin(), SpriteList.end(), [layerId](const Sprite& sprite)
        {
            return sprite.SpriteLayer == layerId;
        });
}

void SpriteSystem::DestroySprite(uint32 spriteId)
{
    auto it = std::find_if(SpriteList.begin(), SpriteList.end(), [spriteId](const Sprite& sprite) 
        { 
            return sprite.SpriteId == spriteId; 
        });

    if (it != SpriteList.end())
    {
        Sprite& sprite = *it;
        if (sprite.SpriteInstanceId != UINT32_MAX)
        {
            memoryPoolSystem.FreeObject(kSpriteInstanceBuffer, sprite.SpriteInstanceId);
            sprite.SpriteInstanceId = UINT32_MAX;
        }
        sprite.SpriteAlive = false;
        SpriteListDirty = true;
    }
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