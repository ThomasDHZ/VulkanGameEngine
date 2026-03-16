#include "pch.h"
#include "SpriteSystem.h"
#include "LevelSystem.h"

SpriteSystem& spriteSystem = SpriteSystem::Get();

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
    Sprite sprite = levelSystem.EntityRegistry.emplace<Sprite>(gameObject.GameObjectComponents, Sprite
        {
            .GameObjectId = gameObject.GameObjectId,
            .SpriteInstanceId = memoryPoolSystem.AllocateObject(kSpriteInstanceBuffer),
            .CurrentAnimationId = 0,
            .CurrentFrame = 0,
            .SpriteLayer = FindSpriteVram(spriteVramId).SpriteLayer,
            .FlipSprite = ivec2(0),
            .SpriteVramId = spriteVramId,
            .CurrentFrameTime = 0.0f
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
    if (SpriteVramExists(vramId))
    {
        return vramId;
    }

    VramSpriteGuid materialId = VramSpriteGuid(json["MaterialId"].get<String>().c_str());
    const Material& material = materialSystem.FindMaterial(materialId);
    const Texture& texture = textureSystem.FindTexture(material.AlbedoDataId);
    spriteSystem.SpriteAnimationMap[vramId] = LoadSpriteAnimations(spriteVramPath.c_str());
    spriteSystem.SpriteVramList.emplace_back(LoadSpriteVRAM(spriteVramPath.c_str(), material, texture));
    return vramId;
}

SpriteVram SpriteSystem::LoadSpriteVRAM(const char* spritePath, const Material& material, const Texture& texture)
{
    nlohmann::json json = fileSystem.LoadJsonFile(spritePath);
    ivec2 spritePixelSize = ivec2{ json["SpritePixelSize"][0], json["SpritePixelSize"][1] };
    ivec2 spriteCells = ivec2(texture.width / spritePixelSize.x, texture.height / spritePixelSize.y);
    ivec2 spriteScale = ivec2{ json["SpriteScale"][0], json["SpriteScale"][1] };

    return SpriteVram
    {
        .VramSpriteID = VkGuid(json["VramSpriteId"].get<String>().c_str()),
        .SpriteMaterialID = material.MaterialGuid,
        .SpriteLayer = json["SpriteLayer"],
        .SpriteColor = vec4{ json["SpriteColor"][0], json["SpriteColor"][1], json["SpriteColor"][2], json["SpriteColor"][3] },
        .SpritePixelSize = ivec2{ json["SpritePixelSize"][0], json["SpritePixelSize"][1] },
        .SpriteScale = ivec2{ json["SpriteScale"][0], json["SpriteScale"][1] },
        .SpriteCells = ivec2(texture.width / spritePixelSize.x, texture.height / spritePixelSize.y),
        .SpriteUVSize = vec2(1.0f / (float)spriteCells.x, 1.0f / (float)spriteCells.y),
        .SpriteSize = vec2(spritePixelSize.x * spriteScale.x, spritePixelSize.y * spriteScale.y),
    };
}

Vector<Animation2D> SpriteSystem::LoadSpriteAnimations(const char* spritePath)
{
    Vector<Animation2D> animationList;
    nlohmann::json json = fileSystem.LoadJsonFile(spritePath);
    for (size_t x = 0; x < json["AnimationList"].size(); ++x)
    {
        Vector<ivec2> spriteFrameList;
        for (size_t y = 0; y < json["AnimationList"][x]["FrameList"].size(); ++y)
        {
            ivec2 frame =
            {
                json["AnimationList"][x]["FrameList"][y][0].get<float>(),
                json["AnimationList"][x]["FrameList"][y][1].get<float>()
            };
            spriteFrameList.emplace_back(frame);
        }

        Animation2D animation =
        {
            .AnimationId = json["AnimationList"][x]["AnimationId"].get<uint>(),
            .FrameList = spriteFrameList,
            .FrameHoldTime = json["AnimationList"][x]["FrameHoldTime"].get<float>(),
        };
        animationList.emplace_back(animation);
    }

    return animationList;
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

            GameObject& gameObject = gameObjectSystem.FindGameObject(sprite.GameObjectId);

            mat4 spriteMatrix = mat4(1.0f);
            spriteMatrix = glm::translate(spriteMatrix, vec3(transform.GameObjectPosition.x, transform.GameObjectPosition.y, 0.0f));
            spriteMatrix = glm::rotate(spriteMatrix, glm::radians(transform.GameObjectRotation.x), vec3(1.0f, 0.0f, 0.0f));
            spriteMatrix = glm::rotate(spriteMatrix, glm::radians(transform.GameObjectRotation.y), vec3(0.0f, 1.0f, 0.0f));
            spriteMatrix = glm::rotate(spriteMatrix, glm::radians(0.0f), vec3(0.0f, 0.0f, 1.0f));
            spriteMatrix = glm::scale(spriteMatrix, vec3(transform.GameObjectScale.x, transform.GameObjectScale.y, 1.0f));
            spriteInstance.SpritePosition = transform.GameObjectPosition;
            spriteInstance.InstanceTransform = spriteMatrix;
        
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
        entries.push_back({ entity, sprite.SpriteLayer });

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

bool SpriteSystem::SpriteVramExists(const VkGuid& vramId)
{
    return std::any_of(spriteSystem.SpriteVramList.begin(), spriteSystem.SpriteVramList.end(), [vramId](const SpriteVram& sprite) { return sprite.VramSpriteID == vramId; });
}

void SpriteSystem::Destroy(Sprite& sprite)
{

}