#include "SpriteSystem.h"
#include "LevelSystem.h"

SpriteSystem& spriteSystem = SpriteSystem::Get();

void SpriteSystem::AddSpriteBatchLayer()
{
    meshSystem.CreateSpriteLayer(SpriteMeshId);
    SpriteLayerList.emplace_back(SpriteLayer
        {
            .InstanceCount = 0,
            .StartInstanceIndex = 0,
            .SpriteDrawLayer = UINT32_MAX,
        });
}

void SpriteSystem::CreateSprite(entt::entity& gameObjectId, VkGuid& spriteVramId)
{
    Sprite sprite = gameObjectSystem.EntityRegistry.emplace<Sprite>(gameObjectId, Sprite
        {
            .GameObjectId = gameObjectId,
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

VramSpriteGuid SpriteSystem::LoadSpriteVRAM(const nlohmann::json& json)
{
    VkGuid materialId = VkGuid(json["GameObjectMaterial"]["MaterialId"].get<String>());
    nlohmann::json gameObjectSpriteJson = json["GameObjectSprite"];
    VramSpriteGuid vramId = VramSpriteGuid(gameObjectSpriteJson["VramSpriteId"].get<String>().c_str());
    if (SpriteVramExists(vramId))
    {
        return vramId;
    }

    const Material& material  = materialSystem.FindMaterial(materialId);
    const Texture& texture    = textureSystem.FindTexture(material.AlbedoDataId);

    ivec2 spritePixelSize = ivec2{ gameObjectSpriteJson["SpritePixelSize"][0], gameObjectSpriteJson["SpritePixelSize"][1] };
    ivec2 spriteCells     = ivec2(texture.width / spritePixelSize.x, texture.height / spritePixelSize.y);
    ivec2 spriteScale     = ivec2{ gameObjectSpriteJson["SpriteScale"][0], gameObjectSpriteJson["SpriteScale"][1] };

    if(gameObjectSpriteJson.contains("AnimationList")) SpriteAnimationMap[vramId] = LoadSpriteAnimations(gameObjectSpriteJson);
    SpriteVramList.emplace_back(SpriteVram
    {
        .VramSpriteID = VkGuid(gameObjectSpriteJson["VramSpriteId"].get<String>().c_str()),
        .SpriteMaterialID = material.MaterialGuid,
        .SpriteLayer = gameObjectSpriteJson["SpriteLayer"],
        .SpriteColor = vec4{ gameObjectSpriteJson["SpriteColor"][0], gameObjectSpriteJson["SpriteColor"][1], gameObjectSpriteJson["SpriteColor"][2], gameObjectSpriteJson["SpriteColor"][3] },
        .SpritePixelSize = ivec2{ gameObjectSpriteJson["SpritePixelSize"][0], gameObjectSpriteJson["SpritePixelSize"][1] },
        .SpriteScale = ivec2{ gameObjectSpriteJson["SpriteScale"][0], gameObjectSpriteJson["SpriteScale"][1] },
        .SpriteCells = ivec2(texture.width / spritePixelSize.x, texture.height / spritePixelSize.y),
        .SpriteUVSize = vec2(1.0f / (float)spriteCells.x, 1.0f / (float)spriteCells.y),
        .SpriteSize = vec2(spritePixelSize.x * spriteScale.x, spritePixelSize.y * spriteScale.y),
    });
}

Vector<Animation2D> SpriteSystem::LoadSpriteAnimations(const nlohmann::json& json)
{
    Vector<Animation2D> animationList;
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

    if (animationList.empty())
    {
        Animation2D animation =
        {
            .AnimationId = 0,
            .FrameList = Vector<ivec2>{ ivec2(0,0) },
            .FrameHoldTime = 0.0f,
        };
        animationList.emplace_back(animation);
    }
    return animationList;
}


void SpriteSystem::Update(const float& deltaTime)
{
    SortSpriteLayers();
    auto view = gameObjectSystem.EntityRegistry.view<GameObject, Sprite, Transform2DComponent>();
    for (auto [entity, gameObject, sprite, transform] : view.each())
    {
        const auto& vram = FindSpriteVram(sprite.SpriteVramId);
        SpriteInstance& spriteInstance = memoryPoolSystem.UpdateSpriteInstance(sprite.SpriteInstanceId);

        mat4 spriteMatrix = mat4(1.0f);
        spriteMatrix = glm::translate(spriteMatrix, vec3(transform.GameObjectPosition.x, transform.GameObjectPosition.y, 0.0f));
        spriteMatrix = glm::rotate(spriteMatrix, glm::radians(transform.GameObjectRotation.x), vec3(1.0f, 0.0f, 0.0f));
        spriteMatrix = glm::rotate(spriteMatrix, glm::radians(transform.GameObjectRotation.y), vec3(0.0f, 1.0f, 0.0f));
        spriteMatrix = glm::rotate(spriteMatrix, glm::radians(0.0f), vec3(0.0f, 0.0f, 1.0f));
        spriteMatrix = glm::scale(spriteMatrix, vec3(transform.GameObjectScale.x, transform.GameObjectScale.y, 1.0f));

        spriteInstance.SpritePosition = transform.GameObjectPosition;
        spriteInstance.InstanceTransform = spriteMatrix;
        spriteInstance.MaterialId = materialSystem.FindMaterialPoolIndex(vram.SpriteMaterialID);
        spriteInstance.SpriteSize = vram.SpriteSize;
        spriteInstance.FlipSprite = sprite.FlipSprite;
        //spriteInstance.Color = materialSystem.FindMaterialPoolIndex(vram.SpriteMaterialID);
        spriteInstance.SpriteId = static_cast<uint>(entity);

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
    auto view = gameObjectSystem.EntityRegistry.view<Sprite, Transform2DComponent>();

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
        auto [sprite, transform] = gameObjectSystem.EntityRegistry.get<Sprite, Transform2DComponent>(entry.entity);
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
    if (sprite->CurrentAnimationId == spriteAnimationEnum) return;

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